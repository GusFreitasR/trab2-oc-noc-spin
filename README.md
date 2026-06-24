# Network-on-Chip (NoC) SPIN Simulator in SystemC

Um simulador de ciclo exato desenvolvido em SystemC que modela a arquitetura de Rede-em-Chip (NoC) SPIN baseada numa topologia de Árvore Gorda Quaternária (Fat-Tree). O projeto implementa controlo de fluxo baseado em créditos, comutação Wormhole e arbitragem probabilística híbrida com roteamento adaptativo.

---

## 🚀 Funcionalidades

O simulador modela o comportamento de hardware ao nível de transferência de registos (RTL/Cycle-Accurate), incorporando os seguintes mecanismos arquiteturais:

*   **Topologia Fat-Tree Quaternária:** Rede estruturada de 2 níveis contendo 8 roteadores (4 Roteadores Folha no Nível 1 e 4 Roteadores Raiz no Nível 2) interligando um ecossistema de 16 Terminais de processamento independentes.
*   **Comutação Wormhole (Wormhole Switching):** Mensagens grandes divididas em unidades elementares chamadas *Flits* (`FLIT_HEADER`, `FLIT_PAYLOAD`, `FLIT_TAIL`). O *Header* aloca a rota e tranca o canal de comunicação, permitindo que o corpo do pacote veja em fila indiana.
*   **Controlo de Fluxo Baseado em Créditos:** Prevenção de perda de dados e transbordo de memória através de contadores de crédito. Um módulo só transmite se o buffer FIFO de entrada do roteador vizinho (capacidade de 4 posições) tiver espaço garantido.
*   **Árbitro Probabilístico Híbrido:** Lógica anti-*starvation* baseada nas especificações da rede SPIN. O roteador roda uma roleta probabilística a cada ciclo para reordenar a prioridade de atendimento dos canais, mitigando congestionamentos permanentes:
    *   **50% de chance:** Prioridade para tráfego descendo da Raiz (Portas UP).
    *   **30% de chance:** Prioridade para tráfego subindo da Folha (Portas DOWN).
    *   **15% de chance:** Intercalado com leve vantagem para UP.
    *   **5% de chance:** Intercalado com leve vantagem para DOWN.
*   **Roteamento Adaptativo e Determinístico:**
    *   *Subida (UP):* Dinâmico e adaptativo. O Roteador Folha avalia as suas 4 saídas superiores e escolhe o caminho com maior número de créditos livres.
    *   *Descida (DOWN):* Estrito e determinístico, calculado via ID lógico do terminal de destino (`dest / 4` e `dest % 4`).

---

## 📂 Estrutura do Projeto

A separação estrita entre declarações (`.h`) e implementações (`.cpp`) garante a modularidade e legibilidade do código:

```
trab2-oc-noc-spin
├── CMakeLists.txt      # Script de automação de compilação CMake
├── main.cpp            # Testbench principal e geração de estímulos
├── rspin_router.h      # Declaração do Roteador RSPIN (Máquina Wormhole)
├── rspin_router.cpp    # Implementação da lógica de arbitragem e roteamento
├── spin_fat_tree.h     # Declaração do Módulo Top-Level da NoC
├── spin_fat_tree.cpp   # Conexões de malha, barramentos de dados e créditos
├── spin_flit.h         # Estrutura do pacote e tipos de Flit
├── terminal.h          # Declaração do injetor/recetor de tráfego
└── terminal.cpp        # Geração da sequência Wormhole e consumo de flits
```

---

## 🛠️ Requisitos e Instalação

### Pré-requisitos

*   **Sistema Operativo:** Linux (Fedora, Ubuntu ou similar)
*   **Compilador:** GCC/G++ suportando o padrão C++17
*   **Automação:** CMake (versão mínima 3.10) e Ninja/Make
*   **Biblioteca:** Accellera SystemC (versão 3.0.2 instalada em `/usr/local/systemc-3.0.2`)

### Instruções de Compilação via Linha de Comando

1.  Clone o repositório para a sua máquina local:
    ```bash
    git clone https://github.com/seu-usuario/trab2-oc-noc-spin.git
    cd trab2-oc-noc-spin
    ```
2.  Crie um diretório para a build e execute o CMake:
    ```bash
    mkdir build && cd build
    cmake ..
    ```
3.  Compile o projeto:
    ```bash
    make -j$(nproc)
    ```

---

## 🎯 Instruções de Teste e Execução

Para rodar a simulação a partir do executável compilado, execute o seguinte comando no seu terminal dentro da pasta de `build`:

```bash
./trab2_oc_noc_spin
```

### Configurando Cenários de Tráfego

A configuração de quais terminais vão comunicar é feita diretamente no ficheiro `main.cpp`. Pode modificar a linha de tráfego para testar caminhos locais (mesmo roteador folha) ou caminhos longos (que exigem subir até a raiz):

```cpp
// main.cpp
// Parâmetros: configure_traffic(Origem [0-15], Destino [0-15])
my_noc.configure_traffic(1, 14);
```