#include "rspin_router.h"
#include <cstdlib>

RSPIN_Router::RSPIN_Router(sc_module_name name, int id, int lvl)
    : sc_module(name), my_id(id), level(lvl) {
    SC_METHOD(routing_logic);
    sensitive << clk.pos() << rst;
}

void RSPIN_Router::routing_logic() {
    if (rst.read()) {
        for(int i=0; i<4; i++) {
            up_out[i].write(SpinFlit()); down_out[i].write(SpinFlit());
            up_credit_out[i].write(false); down_credit_out[i].write(false);
            credits_up_out[i] = 4; credits_down_out[i] = 4;
            out_up_busy[i] = false; out_down_busy[i] = false;

            while(!in_buf_up[i].empty()) in_buf_up[i].pop();
            while(!in_buf_down[i].empty()) in_buf_down[i].pop();
        }
        for(int i=0; i<8; i++) input_locks[i] = {DIR_NONE, -1};
    } else {
        // 1. Processa créditos recebidos
        for(int i=0; i<4; i++) {
            if(up_credit_in[i].read()) credits_up_out[i]++;
            if(down_credit_in[i].read()) credits_down_out[i]++;
        }

        // 2. Lê Flits para os Buffers
        for(int i=0; i<4; i++) {
            if(up_in[i].read().valid) in_buf_up[i].push(up_in[i].read());
            if(down_in[i].read().valid) in_buf_down[i].push(down_in[i].read());
        }

        SpinFlit next_up_out[4]; SpinFlit next_down_out[4];
        bool send_credit_up[4] = {false, false, false, false};
        bool send_credit_down[4] = {false, false, false, false};

        // =================================================================
        // 3. O ÁRBITRO PROBABILÍSTICO (A Roleta de Prioridades)
        // =================================================================
        int roleta = rand() % 100;
        int ordem_avaliacao[8];
        std::string nome_roleta;

        // Portas 0-3 são DOWN (sobem da folha), Portas 4-7 são UP (descem da raiz)
        if (roleta < 50) {
            // 50% de chance: Tráfego descendo da Raiz (UP)
            int seq[8] = {4, 5, 6, 7, 0, 1, 2, 3};
            for(int i=0; i<8; i++) ordem_avaliacao[i] = seq[i];
            nome_roleta = "50% (Prioridade UP)";
        } else if (roleta < 80) {
            // 30% de chance: Tráfego subindo da Folha (DOWN)
            int seq[8] = {0, 1, 2, 3, 4, 5, 6, 7};
            for(int i=0; i<8; i++) ordem_avaliacao[i] = seq[i];
            nome_roleta = "30% (Prioridade DOWN)";
        } else if (roleta < 95) {
            // 15% de chance: Intercalado com leve vantagem para UP
            int seq[8] = {4, 0, 5, 1, 6, 2, 7, 3};
            for(int i=0; i<8; i++) ordem_avaliacao[i] = seq[i];
            nome_roleta = "15% (Misto UP)";
        } else {
            // 5% de chance: Intercalado com leve vantagem para DOWN
            int seq[8] = {0, 4, 1, 5, 2, 6, 3, 7};
            for(int i=0; i<8; i++) ordem_avaliacao[i] = seq[i];
            nome_roleta = "5% (Misto DOWN)";
        }

        // 4. Processa o Roteamento seguindo a ordem sorteada pela Roleta
        for(int idx = 0; idx < 8; idx++) {
            int buf_id = ordem_avaliacao[idx];
            bool is_down_port = (buf_id < 4);
            int local_port = is_down_port ? buf_id : buf_id - 4;
            std::queue<SpinFlit>* my_q = is_down_port ? &in_buf_down[local_port] : &in_buf_up[local_port];

            if (my_q->empty()) continue;

            SpinFlit f = my_q->front();

            // A) FASE DE ALOCAÇÃO (Apenas o HEADER)
            if (f.type == FLIT_HEADER && input_locks[buf_id].dir == DIR_NONE) {
                int folha_dest = f.dest / 4;
                int term_dest  = f.dest % 4;
                bool needs_to_go_up = (level == 1 && folha_dest != my_id);

                // Mostra no terminal que o Árbitro interveio na alocação!
                std::cout << "  [ARBITRO] Roteador Nivel " << level << " ID " << my_id
                          << " | caiu em " << nome_roleta << " -> Atendendo Entrada " << buf_id << std::endl;

                if (needs_to_go_up) {
                    // ROTEAMENTO ADAPTATIVO
                    int best_port = -1;
                    int max_credits = -1;
                    for(int p=0; p<4; p++) {
                        if(!out_up_busy[p] && credits_up_out[p] > max_credits) {
                            max_credits = credits_up_out[p];
                            best_port = p;
                        }
                    }
                    if (best_port != -1) {
                        input_locks[buf_id] = {DIR_UP, best_port};
                        out_up_busy[best_port] = true; // TRANCADO!
                        std::cout << "      -> [HW] Roteamento Adaptativo trancou porta UP " << best_port << std::endl;
                    }
                } else {
                    // ROTEAMENTO DETERMINÍSTICO
                    int target = (level == 1) ? term_dest : folha_dest;
                    if (!out_down_busy[target]) {
                        input_locks[buf_id] = {DIR_DOWN, target};
                        out_down_busy[target] = true; // TRANCADO!
                    }
                }
            }

            // B) FASE DE TRANSMISSÃO
            if (input_locks[buf_id].dir != DIR_NONE) {
                Dir d = input_locks[buf_id].dir;
                int p = input_locks[buf_id].port;
                std::string tipo_str = (f.type == FLIT_HEADER) ? "[HEADER] " : (f.type == FLIT_TAIL) ? "[TAIL]   " : "[PAYLOAD]";

                if (d == DIR_UP && credits_up_out[p] > 0) {
                    next_up_out[p] = f;
                    credits_up_out[p]--;
                    my_q->pop();
                    if(is_down_port) send_credit_down[local_port] = true; else send_credit_up[local_port] = true;

                    std::cout << "    >> @ " << sc_time_stamp() << " | Roteador Nivel " << level << " (ID " << my_id
                              << ") despachou " << tipo_str << " -> Porta UP " << p << std::endl;

                    if (f.type == FLIT_TAIL) {
                        input_locks[buf_id] = {DIR_NONE, -1};
                        out_up_busy[p] = false;
                    }
                }
                else if (d == DIR_DOWN && credits_down_out[p] > 0) {
                    next_down_out[p] = f;
                    credits_down_out[p]--;
                    my_q->pop();
                    if(is_down_port) send_credit_down[local_port] = true; else send_credit_up[local_port] = true;

                    std::cout << "    >> @ " << sc_time_stamp() << " | Roteador Nivel " << level << " (ID " << my_id
                              << ") despachou " << tipo_str << " -> Porta DOWN " << p << std::endl;

                    if (f.type == FLIT_TAIL) {
                        input_locks[buf_id] = {DIR_NONE, -1};
                        out_down_busy[p] = false; // Rota liberta!
                    }
                }
            }
        }

        // 5. Escreve
        for(int i=0; i<4; i++) {
            up_out[i].write(next_up_out[i]);
            down_out[i].write(next_down_out[i]);
            up_credit_out[i].write(send_credit_up[i]);
            down_credit_out[i].write(send_credit_down[i]);
        }
    }
}