#include<systemc.h>
#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<nana/gui.hpp>
#include<nana/gui/widgets/button.hpp>
#include<nana/gui/widgets/menubar.hpp>
#include<nana/gui/widgets/group.hpp>
#include<nana/gui/widgets/textbox.hpp>
#include<nana/gui/filebox.hpp>
#include "top.hpp"
#include "gui.hpp"

#include <bits/stdc++.h>
#include <string>
#include <stdexcept>

#include "bpb.h"

using std::string;
using std::vector;
using std::map;
using std::fstream;

// defaults amount reserve stations
#define N_RS_ADD 3
#define N_RS_MUL 2
#define N_RS_LS 2

#ifndef USE_BPB
float
predictor_get_hit_rate( void );
#endif

void dump_regs(nana::listbox::cat_proxy reg_gui, nana::grid &memory, std::string regi_path, std::string regf_path, std::string mem_path);
string get_file_name(string path, int idx=1);
bool diff(nana::listbox::cat_proxy reg_gui, nana::grid &memory, string regi_path, string regpf_path, string mem_path);

void show_metrics( top & );

static unsigned int cpu_freq = 1000; // default 1Ghz used to compute metrics

int sc_main(int argc, char *argv[])
{
    using namespace nana;
    vector<string> instruction_queue;

#ifdef USE_BPB
    unsigned int len_bpb = 0; // 0 equal usage default lenght bpb
#endif

    int nadd = N_RS_ADD;
    int nmul = N_RS_MUL;
    int nls = N_RS_LS;

    string regi_file_name   = "NA";
    string regfp_file_name  = "NA";
    string memory_file_name = "NA";

    std::vector<int> sizes;
    bool spec = false;
    bool fila = false;
    ifstream inFile;
    form fm(API::make_center(1024,700));
    place plc(fm);
    place upper(fm);
    place lower(fm);
    listbox table(fm);
    listbox reg(fm);
    listbox instruct(fm);
    listbox rob(fm);
    menubar mnbar(fm);
    button start(fm);
    button btn_full_execute(fm);
    button clock_control(fm);
    button exit(fm);
    group clock_group(fm);
    label clock_count(clock_group);
    fm.caption("TFSim");
    clock_group.caption("Ciclo de Clock");
    clock_group.div("count");
    grid memory(fm,rectangle(),10,50);
    // Tempo de latencia de uma instrucao
    // Novas instrucoes devem ser inseridas manualmente aqui
    map<string,int> instruct_time{{"DADD",4},{"DADDI",4},{"DSUB",6},{"DSUBI",6},{"DMUL",10},{"DDIV",16},{"MEM",2}};
    top top1("top");
    start.caption("Start");
    clock_control.caption("Next Cycle");
    btn_full_execute.caption("Full Execute");
    exit.caption("Exit");
    plc["rst"] << table;
    plc["btns"] << start << clock_control << btn_full_execute << exit;
    plc["memor"] << memory;
    plc["regs"] << reg;
    plc["rob"] << rob;
    plc["instr"] << instruct;
    plc["clk_c"] << clock_group;
    clock_group["count"] << clock_count;
    clock_group.collocate();

    spec = false;
    set_spec(plc,spec);
    plc.collocate();

    mnbar.push_back("Opções");
    menu &op = mnbar.at(0);
    menu::item_proxy spec_ip = op.append("Especulação",[&](menu::item_proxy &ip)
    {
        if(ip.checked())
            spec = true;
        else
            spec = false;
        set_spec(plc,spec);
    });
    op.check_style(0,menu::checks::highlight);
    op.append("Modificar valores...");
    auto sub = op.create_sub_menu(1);
    sub->append("Número de Estações de Reserva",[&](menu::item_proxy ip)
    {
        inputbox ibox(fm,"","Quantidade de Estações de Reserva");
        inputbox::integer add("ADD/SUB",nadd,1,10,1);
        inputbox::integer mul("MUL/DIV",nmul,1,10,1);
        inputbox::integer sl("LOAD/STORE",nls,1,10,1);
        if(ibox.show_modal(add,mul,sl))
        {
            nadd = add.value();
            nmul = mul.value();
            nls = sl.value();
        }
    });
    // Menu de ajuste dos tempos de latencia na interface
    // Novas instrucoes devem ser adcionadas manualmente aqui
    sub->append("Tempos de latência", [&](menu::item_proxy &ip)
    {
        inputbox ibox(fm,"","Tempos de latência para instruções");
        inputbox::text dadd_t("DADD",std::to_string(instruct_time["DADD"]));
        inputbox::text daddi_t("DADDI",std::to_string(instruct_time["DADDI"]));
        inputbox::text dsub_t("DSUB",std::to_string(instruct_time["DSUB"]));
        inputbox::text dsubi_t("DSUBI",std::to_string(instruct_time["DSUBI"]));
        inputbox::text dmul_t("DMUL",std::to_string(instruct_time["DMUL"]));
        inputbox::text ddiv_t("DDIV",std::to_string(instruct_time["DDIV"]));
        inputbox::text mem_t("Load/Store",std::to_string(instruct_time["MEM"]));
        if(ibox.show_modal(dadd_t,daddi_t,dsub_t,dsubi_t,dmul_t,ddiv_t,mem_t))
        {
            instruct_time["DADD"] = std::stoi(dadd_t.value());
            instruct_time["DADDI"] = std::stoi(daddi_t.value());
            instruct_time["DSUB"] = std::stoi(dsub_t.value());
            instruct_time["DSUBI"] = std::stoi(dsubi_t.value());
            instruct_time["DMUL"] = std::stoi(dmul_t.value());
            instruct_time["DDIV"] = std::stoi(ddiv_t.value());
            instruct_time["MEM"] = std::stoi(mem_t.value());
        }
    });
    sub->append("Fila de instruções", [&](menu::item_proxy &ip)
    {
        filebox fb(0,true);
        inputbox ibox(fm,"Localização do arquivo com a lista de instruções:");
        inputbox::path caminho("",fb);
        if(ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if(!add_instructions(inFile,instruction_queue,instruct))
                show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
            else
                fila = true;
        }
    });
    sub->append("Valores de registradores inteiros",[&](menu::item_proxy &ip)
    {
        filebox fb(0,true);
        inputbox ibox(fm,"Localização do arquivo de valores de registradores inteiros:");
        inputbox::path caminho("",fb);
        if(ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if(!inFile.is_open())
                show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
            else
            {
                auto reg_gui = reg.at(0);
                int value,i = 0;
                while(inFile >> value && i < 32)
                {
                    reg_gui.at(i).text(1,std::to_string(value));
                    i++;
                }
                for(; i < 32 ; i++)
                    reg_gui.at(i).text(1,"0");
                inFile.close();
            }
        }
    });
    sub->append("Valores de registradores PF",[&](menu::item_proxy &ip)
    {
        filebox fb(0,true);
        inputbox ibox(fm,"Localização do arquivo de valores de registradores PF:");
        inputbox::path caminho("",fb);
        if(ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if(!inFile.is_open())
                show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
            else
            {
                auto reg_gui = reg.at(0);
                int i = 0;
                float value;
                while(inFile >> value && i < 32)
                {
                    reg_gui.at(i).text(4,std::to_string(value));
                    i++;
                }
                for(; i < 32 ; i++)
                    reg_gui.at(i).text(4,"0");
                inFile.close();
            }
        }
    });
    sub->append("Valores de memória",[&](menu::item_proxy &ip)
    {
        filebox fb(0,true);
        inputbox ibox(fm,"Localização do arquivo de valores de memória:");
        inputbox::path caminho("",fb);
        if(ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if(!inFile.is_open())
                show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
            else
            {
                int i = 0;
                int value;
                while(inFile >> value && i < 500)
                {
                    memory.Set(i,std::to_string(value));
                    i++;
                }
                for(; i < 500 ; i++)
                {
                    memory.Set(i,"0");
                }
                inFile.close();
            }
        }
    });
    op.append("Verificar conteúdo...");
    auto new_sub = op.create_sub_menu(2);
    new_sub->append("Valores de registradores",[&](menu::item_proxy &ip) {
        filebox fb(0,true);
        inputbox ibox(fm,"Localização do arquivo de valores de registradores:");
        inputbox::path caminho("",fb);
        if(ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if(!inFile.is_open())
                show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
            else
            {
                string str;
                int reg_pos,i;
                bool is_float, ok;
                ok = true;
                while(inFile >> str)
                {
                    if(str[0] == '$')
                    {
                        if(str[1] == 'f')
                        {
                            i = 2;
                            is_float = true;
                        }
                            else
                        {
                            i = 1;
                            is_float = false;
                        }
                        reg_pos = std::stoi(str.substr(i,str.size()-i));

                        auto reg_gui = reg.at(0);
                        string value;
                        inFile >> value;
                        if(!is_float)
                        {
                            if(reg_gui.at(reg_pos).text(1) != value)
                            {
                                ok = false;
                                break;
                            }
                        }
                        else
                        {
                            if(std::stof(reg_gui.at(reg_pos).text(4)) != std::stof(value))
                            {
                                ok = false;
                                break;
                            }
                        }
                    }
                }
                inFile.close();
                msgbox msg("Verificação de registradores");
                if(ok)
                {
                    msg << "Registradores especificados apresentam valores idênticos!";
                    msg.icon(msgbox::icon_information);
                }
                else
                {
                    msg << "Registradores especificados apresentam valores distintos!";
                    msg.icon(msgbox::icon_error);
                }
                msg.show();
            }
        }
    });
    new_sub->append("Valores de memória",[&](menu::item_proxy &ip) {
        filebox fb(0,true);
        inputbox ibox(fm,"Localização do arquivo de valores de memória:");
        inputbox::path caminho("",fb);
        if(ibox.show_modal(caminho))
        {
            auto path = caminho.value();
            inFile.open(path);
            if(!inFile.is_open())
                show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
            else
            {
                string value;
                bool ok;
                ok = true;
                for(int i = 0 ; i < 500 ; i++)
                {
                    inFile >> value;
                    if(std::stoi(memory.Get(i)) != (int)std::stol(value,nullptr,16))
                    {
                        ok = false;
                        break;
                    }
                }
                inFile.close();
                msgbox msg("Verificação de memória");
                if(ok)
                {
                    msg << "Endereços de memória especificados apresentam valores idênticos!";
                    msg.icon(msgbox::icon_information);
                }
                else
                {
                    msg << "Endereços de memória especificados apresentam valores distintos!";
                    msg.icon(msgbox::icon_error);
                }
                msg.show();
            }
        }
    });
    op.append("Benchmarks");
    auto bench_sub = op.create_sub_menu(3);
    bench_sub->append("Fibonacci",[&](menu::item_proxy &ip){
        string path = "in/benchmarks/fibonacci.txt";
        inFile.open(path);
        if(!add_instructions(inFile,instruction_queue,instruct))
            show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
        else
            fila = true;
    });
    bench_sub->append("Busca em Vetor",[&](menu::item_proxy &ip){
        string path = "in/benchmarks/vector_search.txt";
        inFile.open(path);
        if(!add_instructions(inFile,instruction_queue,instruct))
            show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
        else
            fila = true;
    });
    bench_sub->append("Stall por Divisão",[&](menu::item_proxy &ip){
        string path = "in/benchmarks/division_stall.txt";
        inFile.open(path);
        if(!add_instructions(inFile,instruction_queue,instruct))
            show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
        else
            fila = true;
    });
    bench_sub->append("Stress de Memória (Stores)",[&](menu::item_proxy &ip){
        string path = "in/benchmarks/store_stress.txt";
        inFile.open(path);
        if(!add_instructions(inFile,instruction_queue,instruct))
            show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
        else
            fila = true;
    });
    bench_sub->append("Stall por hazard estrutural (Adds)",[&](menu::item_proxy &ip){
        string path = "in/benchmarks/res_stations_stall.txt";
        inFile.open(path);
        if(!add_instructions(inFile,instruction_queue,instruct))
            show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
        else
            fila = true;
    });
    vector<string> columns = {"#","Name","Busy","Op","Vj","Vk","Qj","Qk","A"};
    for(unsigned int i = 0 ; i < columns.size() ; i++)
    {
        table.append_header(columns[i].c_str());
        if(i && i != 3)
            table.column_at(i).width(45);
        else if (i == 3)
            table.column_at(i).width(60);
        else
            table.column_at(i).width(30);
    }
    columns = {"","Value","Qi"};
    sizes = {30,75,40};
    for(unsigned int k = 0 ; k < 2 ; k++)
        for(unsigned int i = 0 ; i < columns.size() ; i++)
        {
            reg.append_header(columns[i]);
            reg.column_at(k*columns.size() + i).width(sizes[i]);
        }

    auto reg_gui = reg.at(0);
    for(int i = 0 ; i < 32 ;i++)
    {
        string index = std::to_string(i);
        reg_gui.append("R" + index);
        reg_gui.at(i).text(3,"F" + index);
    }

    columns = {"Instruction","Issue","Execute","Write Result"};
    sizes = {140,60,70,95};
    for(unsigned int i = 0 ; i < columns.size() ; i++)
    {
        instruct.append_header(columns[i]);
        instruct.column_at(i).width(sizes[i]);
    }
    columns = {"Entry","Busy","Instruction","State","Destination","Value"};
    sizes = {45,45,120,120,90,60};
    for(unsigned int i = 0 ; i < columns.size() ; i++)
    {
        rob.append_header(columns[i]);
        rob.column_at(i).width(sizes[i]);
    }

    srand(static_cast <unsigned> (time(0)));
    for(int i = 0 ; i < 32 ; i++)
    {
        if(i)
            reg_gui.at(i).text(1,std::to_string(rand()%100));
        else
            reg_gui.at(i).text(1,std::to_string(0));
        reg_gui.at(i).text(2,"0");
        reg_gui.at(i).text(4,std::to_string(static_cast <float> (rand()) / static_cast <float> (RAND_MAX/100.0)));
        reg_gui.at(i).text(5,"0");
    }
    for(int i = 0 ; i < 500 ; i++)
        memory.Push(std::to_string(rand()%100));
    for(int k = 1; k < argc; k+=2)
    {
        int i;
        if ( '-' != argv[k][0] )
            show_message("Opção inválida",string("Opção \"") + string(argv[k]) + string("\" inválida"));
        else
        {
            char c = argv[k][1];

            switch(c)
            {
                case 'c':
                  if ( !strcmp( argv[k], "-cpu") )
                    {
                      cpu_freq = std::stoi(argv[k+1]);
                    }
                  break;
                case 'q':
                    inFile.open(argv[k+1]);
                    if(!add_instructions(inFile,instruction_queue,instruct))
                        show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
                    else
                        fila = true;
                    break;
                case 'i':

                    inFile.open(argv[k+1]);

                    regi_file_name = argv[k+1];
                    cout << "valor: " << get_file_name(regi_file_name) << " " << regi_file_name  <<endl;

                    int value;
                    i = 0;
                    if(!inFile.is_open())
                        show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
                    else
                    {
                        while(inFile >> value && i < 32)
                        {
                            reg_gui.at(i).text(1,std::to_string(value));
                            i++;
                        }
                        for(; i < 32 ; i++)
                            reg_gui.at(i).text(1,"0");
                        inFile.close();
                    }
                    break;
                case 'f':
                    float value_fp;
                    i = 0;
                    inFile.open(argv[k+1]);

                    regfp_file_name = argv[k+1];

                    if(!inFile.is_open())
                        show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
                    else
                    {
                        while(inFile >> value_fp && i < 32)
                        {
                            reg_gui.at(i).text(4,std::to_string(value_fp));
                            i++;
                        }
                        for(; i < 32 ; i++)
                            reg_gui.at(i).text(4,"0");
                        inFile.close();
                    }
                    break;
                case 'm':
                    inFile.open(argv[k+1]);
                    memory_file_name = argv[k+1];

                    if(!inFile.is_open())
                        show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
                    else
                    {
                        int value;
                        i = 0;
                        while(inFile >> value && i < 500)
                        {
                            memory.Set(i,std::to_string(value));
                            i++;
                        }
                        for(; i < 500 ; i++)
                        {
                            memory.Set(i,"0");
                        }
                        inFile.close();
                    }
                    break;
                case 'r':
                    inFile.open(argv[k+1]);
                    if(!inFile.is_open())
                        show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
                    else
                    {
                        int value;
                        if(inFile >> value && value <= 10 && value > 0)
                            nadd = value;
                        if(inFile >> value && value <= 10 && value > 0)
                            nmul = value;
                        if(inFile >> value && value <= 10 && value > 0)
                            nls = value;
                        inFile.close();
                    }
                    break;
                case 's':
                    spec = true;
                    set_spec(plc,spec);
                    spec_ip.checked(true);
                    k--;
                    break;
                case 'l':
                    inFile.open(argv[k+1]);
                    if(!inFile.is_open())
                        show_message("Arquivo inválido","Não foi possível abrir o arquivo!");
                    else
                    {
                        string inst;
                        int value;
                        while(inFile >> inst)
                        {
                            if(inFile >> value && instruct_time.count(inst))
                                instruct_time[inst] = value;
                        }
                    }
                    inFile.close();
                    break;
#ifdef USE_BPB
                case 'b':
                    if ( !strcmp ( argv[k], "-bpb" ) )
                      len_bpb = std::stoi( argv[ k + 1 ] );

                  break;
#endif
                default:
                    show_message("Opção inválida",string("Opção \"") + string(argv[k]) + string("\" inválida"));
                    break;
            }
        }
    }

    clock_control.enabled(false);
    btn_full_execute.enabled(false);


#ifdef USE_BPB
    if ( !bpb_init( len_bpb ) )
      {
        cerr << "Error init BPB" << endl;
        sc_stop();
        API::exit();
      }

    cout << "Uisng BPB with lenght " << ( len_bpb ? std::to_string(len_bpb) : "default." ) << endl;
#endif

    start.events().click([&]
    {
        if(fila)
        {
            start.enabled(false);
            clock_control.enabled(true);
            btn_full_execute.enabled(true);
            //Desativa os menus apos inicio da execucao
            op.enabled(0,false);
            op.enabled(1,false);
            op.enabled(3,false);

            for(int i = 0 ; i < 6 ; i++)
                sub->enabled(i,false);
            for(int i = 0 ; i < 5 ; i++)
                bench_sub->enabled(i,false);
            if(spec)
                top1.rob_mode(nadd,nmul,nls,instruct_time,instruction_queue,table,memory,reg,instruct,clock_count,rob);
            else
                top1.simple_mode(nadd,nmul,nls,instruct_time,instruction_queue,table,memory,reg,instruct,clock_count);
            sc_start();
        }
        else
            show_message("Fila de instruções vazia","A fila de instruções está vazia. Insira um conjunto de instruções para iniciar.");
    });

    clock_control.events().click([&]
    {
      if ( top1.get_queue().queue_is_empty() &&
           top1.get_rob().rob_is_empty() )
        return;

      if(sc_is_running())
          sc_start();
    });

    btn_full_execute.events().click([&]
    {
      // while queue and rob is not empty, run to the end.
      while( !( top1.get_queue().queue_is_empty() &&
                top1.get_rob().rob_is_empty() ) )
        {
          if(sc_is_running())
              sc_start();
        }

        dump_regs(reg_gui, memory, regi_file_name, regfp_file_name, memory_file_name);

        try{
            diff(reg_gui, memory, regi_file_name, get_file_name(regfp_file_name), get_file_name(memory_file_name));
        }
        catch(int ex){
            show_message("Erro!", "Impossível executar comparação de corretude de saída. Arquivos de comparação inexistentes. Por favor corrija o problema e execute novamente.");
        }

        show_metrics( top1 );

    });

    exit.events().click([]
    {
        sc_stop();
        API::exit();
    });
    fm.show();
    exec();

#ifdef USE_BPB
    bpb_free();
#endif

    return 0;
}

/**
 * @brief Writes integer and floating point precision registers and memory status into its proper files. Uses folder results as storage.
 *
 * @param reg_gui nana listbox handler
 * @param memory nana grid
 */
void dump_regs(nana::listbox::cat_proxy reg_gui, nana::grid &memory, std::string regi_path, std::string regf_path, std::string mem_path){
    int i;
    ofstream out_file;
    std::string new_path;

    string exp_name = get_file_name(regi_path,3);
    string fname = get_file_name(regi_path);
    string::size_type loc = regi_path.find( exp_name, 0 );

    if (loc !=string::npos){
        new_path = "experiments/"+exp_name+"/dump/";
    }

    if(! std::filesystem::exists(new_path)){
        cout << "criando diretório: " << new_path <<endl;
        std::filesystem::create_directory(new_path);
    }

    cout << "Experimento: " << exp_name <<endl;
    out_file.open(new_path+fname);

    cout << "Salvando os registradores e estado da memória" << endl;

    for(i=0; i< 31; i++){
        out_file << reg_gui.at(i).text(1) << " ";
    }
    out_file << reg_gui.at(31).text(1);
    out_file.close();


    fname = get_file_name(regf_path);
    if (fname!="NA"){
        out_file.open(new_path+fname);
        for(i=0; i< 31; i++){
            out_file << std::stof(reg_gui.at(i).text(4)) << " ";
        }
        out_file << reg_gui.at(31).text(4);
        out_file.close();
    }

    fname = get_file_name(mem_path);
    out_file.open(new_path+fname);
    for(i=0; i< 499; i++){
        out_file << memory.Get(i) << " ";
    }
    out_file << memory.Get(i);
    out_file.close();



}

/**
 * @brief Get the file name object removing its path.
 *
 * @param path relative path from where a reg/mem file is stored
 * @param idx defines the relative position of path to return from the end to the beginning 
 * @return string
 */
string get_file_name(string path, int idx){
    std::stringstream orig(path);
    string substr;
    std::vector<string> vec;
    while(orig.good()){
        getline(orig, substr, '/');
        vec.push_back(substr);
    }

    return vec.at(vec.size()-idx);
}

/**
 * @brief Compares registers and memory post processing data with expected values. Expected values must be stored in out subdir. Prints the inconsistency, if any.
 * 
 * @param reg_gui GUI register panel
 * @param memory  GUI memory data panel
 * @param regi_path Relative path to integer registers input file location.
 * @param regfp_path Relative path to floating point registers input file location.
 * @param mem_path Relative path to memory input location.
 * @return true 
 * @return false 
 */
bool diff(nana::listbox::cat_proxy reg_gui, nana::grid &memory, string regi_path, string regfp_path, string mem_path){
    ifstream in_file;
    int value, i=0;
    std::map<string, std::vector<int>> wrong_values;

    std::string new_path;

    string exp_name = get_file_name(regi_path,3);
    string::size_type loc = regi_path.find( exp_name, 0 );

    if (loc !=string::npos){
        new_path = "experiments/"+exp_name+"/out/";
    }

    string new_regi = new_path+get_file_name(regi_path);
    string new_regf = new_path+get_file_name(regfp_path);
    string new_mem = new_path+get_file_name(mem_path);

    cout << "Iniciando comparação com os arquivos de saída:"<< endl;
    cout << new_regi<<endl;
    cout << new_regf<< endl;
    cout << new_mem<<endl;

    //só deve lançar erro caso o arquivo fonte tenha sido informado, mas seu equivalente de caso de teste não tenha sido encontrado.
    if(! ( (std::filesystem::exists(new_regi)  || get_file_name(new_regi) ==  "NA") &&
           (std::filesystem::exists(new_regf) || get_file_name(new_regf) == "NA") &&
           (std::filesystem::exists(new_mem)   || get_file_name(new_mem) == "NA") )){
        throw 1;
    }

    in_file.open(new_regi);

    wrong_values.insert( std::pair<string, std::vector<int>>("regi", std::vector<int>()) );
    wrong_values.insert( std::pair<string, std::vector<int>>("regpf", std::vector<int>()) );
    wrong_values.insert( std::pair<string, std::vector<int>>("mem", std::vector<int>()) );

    if( get_file_name(regi_path) != "NA" ){
        for(i=0; in_file >> value  && i < 32;i++){
            
            if(value != stoi(reg_gui.at(i).text(1)) ){
                wrong_values.at("regi").push_back(i);
            }
        }
    }
    in_file.close();

    if(get_file_name(regfp_path) != "NA"){
        in_file.open(new_mem);
        for(i=0; in_file >> value  && i < 32;i++){
            if(value != stof(reg_gui.at(i).text(4)) ){
                wrong_values.at("regpf").push_back(i);
            }
        }
    }
    in_file.close();

    if(get_file_name(mem_path) != "NA"){
        in_file.open(mem_path);//possível problema com valores em pf na memória
        for(i=0; in_file >> value  && i < 500;i++){
            if(value != stoi(memory.Get(i)) ){
                wrong_values.at("mem").push_back(i);
            }
        }
    }
    in_file.close();

    if(wrong_values.at("regi").empty() && wrong_values.at("regpf").empty() && wrong_values.at("mem").empty()){
        show_message("Sucesso na execução","Todos os valores correspondem ao caso de teste!");
    }
    else{
        string msg;

        auto aux = wrong_values.at("regi");
        int i=0;

        for(auto it = aux.cbegin(); it!=aux.cend(); ++it, i++){
            msg += "R"+std::to_string(*it)+"\n";
        }

        aux = wrong_values.at("regpf");
        for(auto it = aux.cbegin(); it!=aux.cend(); ++it, i++){
            msg += "F"+std::to_string(*it)+"\n";
        }

        aux = wrong_values.at("mem");
        for(auto it = aux.cbegin(); it!=aux.cend(); ++it, i++){
            int abs = *it;
            msg += "M["+std::to_string(abs/50)+"]["+std::to_string(abs%50)+"]\n";
        }

        show_message("Falha na execução","verifique o resultado dos seguintes registradores/endereços de memória: \n"+msg);
    }
    return true;
}

#define MHZ( clock ) ( clock * 1000000U )
#define CYCLE_TIME( clock ) ( 1.0 / MHZ( clock ) )
#define MIPS( clock, cpi ) ( MHZ(clock) / cpi / 1000000U )
#define TO_NS( t ) ( t * 1000000000ULL )

void show_metrics( top & top )
{
  const unsigned int total_cycles = ( sc_time_stamp().value() - 1 ) / 1000U;
  const unsigned int total_instructions = top.get_queue().get_instruction_counter();
  const float cpi = ( float ) total_cycles / total_instructions;

  const unsigned int mips = MIPS( cpu_freq, cpi );

  const float cycle_time = TO_NS( CYCLE_TIME( cpu_freq ) );
  const unsigned int cpu_time = total_cycles * cycle_time;

  cout << std::fixed;
  cout <<
  "\n\n"
  "Metricas:\n" <<
  "Clock da CPU     - " << cpu_freq << " Mhz\n" <<
  "Total ciclos     - " << total_cycles << "\n" <<
  "Total instruções - " << total_instructions << "\n" <<
  "CPI              - " << std::setprecision(2) << cpi << "\n" <<
  "MIPS             - " << mips << "\n" <<
  "Tempo de ciclo   - " << std::setprecision(2) << cycle_time << " ns\n" <<
  "Tempo de CPU     - " << cpu_time << " ns\n"
#ifndef USE_BPB
  "Prediction Sucess rate  - " << std::setprecision(2) << predictor_get_hit_rate() << "%"
#else
  "BPB Sucess rate  - " << std::setprecision(2) << bpb_get_hit_rate() << "%"
#endif
   << endl;
}
