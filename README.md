
# TFSim

Event-driven simulation of the Tomasulo Algorithm using the C++ interface SystemC, with a complete GUI.

The purpose of this project is to ease the teaching of this algorithm, deemed extremely important to the understanding of
out-of-order execution on modern microprocessors.

# Guia de Execução

1. Instale os pacotes necessários para uso da biblioteca Nana
	```apt install -y unzip libx11-dev libxft-dev libasound2-dev libxcursor-dev```
2. Execute o script na raíz do repositório para instalar as dependências do simulador
	`sh get_dep.sh`
3. compile o simulador `make`
4. Execute com `./tfsim`


- Os valores iniciais de registradores e de memória são gerados aleatoriamente para cada execução. É possível alterá-los (assim como inserir a lista de instruções) por linha de comando ou pela barra de opções da interface. Por linha de comando, utilize:


		-  '-q' para fila de instruções
		- '-i' para valores de registradores inteiros (32 valores)
		- '-f' para valores de registradores PF (32 valores)
		- '-m' para valores de memória (500 valores inteiros)
		- '-r' para número de unidades funcionais (3 inteiros, um para cada tipo (ADD,MULT,LOAD/STORE))
		- '-l' para tempo de latência para cada instrução (uma linha para cada instrução, do formato <INSTRUÇÃO> <tempo de latência em ciclos>)
		- '-s' indica que o programa execute em modo de especulação por hardware (com reorder buffer)
		- '-cpu' informa a frequencia da CPU para calculo de metricas, converta a frequencia em mhz.  
		         ex. para informar uma frequencia de 1.5Ghz use `-cpu 1500`
		* O repositório fornece arquivos de teste já preenchidos na pasta 'in'
        * Também são fornecidos benchmarks para testes básicos (ideais para validação da ferramenta), incluidos em in/benchmarks
* Observações:
	- Caso esteja obtendo erro na compilação do simulador devido a biblioteca stdc++fs, utilize a regra nofs:
		```make nofs```
	- Um vídeo de demonstração da execução do simulador pode ser visto [aqui](https://youtu.be/hleCH6yndPY)


Instruções suportadas:

Sem especulação | Com especulação
---| ---|
DADD | BEQ |
DADDI| BNE |
DSUB | BGTZ |
DSUBI| BLTZ |
DMUL | BGEZ |
DDIV| BLEZ |
LD|
SD|

### Branch prediction Buffer (BPB)
 Para utilizar o BPB, compile com
 ```console
 $ CFLAGS=-DUSE_BPB make
 ```    
 ao compilar com essa flag uma nova opção de linha de comando `-bpb`   pode ser utilizada  
 para especificar a quantidade de posições do buffer.  

 Ex.
 ```console
 $ ./tfsim -bpb 8
 ```  
 tera um buffer de 8 posições.  
 Caso a opção não seja utilizada o BPB tera um tamanho padrão de `10` posições.
