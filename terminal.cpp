#include "terminal.h"

Terminal::Terminal(sc_module_name name, int id)
    : sc_module(name), my_id(id), target_id(-1), flits_to_send(0), my_credits(4) {
    SC_METHOD(process_traffic);
    sensitive << clk.pos();
}

void Terminal::set_target(int t_id) {
    target_id = t_id;
    flits_to_send = 4; // 1 Header, 2 Payloads, 1 Tail
}

void Terminal::process_traffic() {
    if (rst.read()) {
        tx.write(SpinFlit());
        credit_out.write(false);
        my_credits = 4;
    } else {

        if (credit_in.read()) { my_credits++; }

        if (flits_to_send > 0 && my_credits > 0) {
            SpinFlit f;
            f.valid = true;
            f.src = my_id;
            f.dest = target_id;

            if (flits_to_send == 4) {
                f.type = FLIT_HEADER;
                f.data = ">> INICIO_DA_MENSAGEM <<";
                std::cout << "\n[ 🚀 PACOTE INICIADO ] @ " << sc_time_stamp() << " | Origem: " << my_id << " Destino: " << target_id << std::endl;
            } else if (flits_to_send == 1) {
                f.type = FLIT_TAIL;
                f.data = ">> FIM_DA_MENSAGEM <<";
            } else {
                f.type = FLIT_PAYLOAD;
                int num_payload = (flits_to_send == 3) ? 1 : 2;
                f.data = "Olá mundo :) #" + std::to_string(num_payload);
            }

            tx.write(f);
            my_credits--;
            flits_to_send--;
        } else {
            tx.write(SpinFlit());


            // RECEPÇÃO DO PACOTE
            SpinFlit rx_f = rx.read();
            bool consume_flit = false;

            if (rx_f.valid) {
                consume_flit = true;

                std::string tipo = (rx_f.type == FLIT_HEADER) ? "[HEADER] " : (rx_f.type == FLIT_TAIL) ? "[TAIL]   " : "[PAYLOAD]";
                std::cout << "    <<< @ " << sc_time_stamp() << " | Terminal " << my_id
                          << " RECEBEU " << tipo << " | Dado: " << rx_f.data << std::endl;

                if(rx_f.type == FLIT_TAIL) {
                    std::cout << "        -> (Pacote completamente recebido e remontado!)\n" << std::endl;
                }
            }

            credit_out.write(consume_flit);
        }
    }
}