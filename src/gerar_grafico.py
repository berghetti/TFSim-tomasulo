import matplotlib.pyplot as plt
import argparse
import os

parser = argparse.ArgumentParser()

parser.add_argument('--experiment_folder', default='../result', type=str)

args = parser.parse_args()

exp_folder = args.experiment_folder

#pega todas as pastas dentro de results
subdirs = [exp_folder+'/'+el for x in os.walk(exp_folder) for el in x[1]]

for subdir in subdirs:
    files = [str(el) for x in os.walk(subdir) for el in x[2] if "result" in str(el)]

    exp_name = subdir.split('/')[-1]

    values_cpi = []
    values_mips = []
    values_tcpu = []
    values_acc = []

    values_x = []
    labels = []
    for file in files:

        st = file.find('_')+1
        end= file.find('.')
        id = file[st:end]
        values_x.append(id)
        labels.append(f'Tamanho do BPB: {id}')

        with open(os.path.join(subdir,file), 'r') as f:
            content = f.read()
            
            st = content.find('CPI')
            st = content.find('-', st)+1
            end = content.find('\n', st)
            value = float(content[st: end])
            
            values_cpi.append(value)
            print(value)

            st = content.find('MIPS')
            st = content.find('-', st)+1
            end = content.find('\n', st)
            value = float(content[st: end])
            
            values_mips.append(value)
            print(value)


            st = content.find('Tempo de CPU')
            st = content.find('-', st)+1
            end = content.find('\n', st)-2
            value = int(content[st: end])
            
            values_tcpu.append(value)
            print(value)


            st = content.find('BPB Sucess')
            st = content.find('-', st)+1
            end = content.find('\n', st)-1
            value = float(content[st: end])
            
            values_acc.append(value)
            print(value)
        
    plt.figure()
    plt.title('Ciclos por instrução (CPI)')
    plt.bar(values_x,values_cpi)
    plt.xlabel('Tamanho do BPB')
    plt.ylabel('CPI')
    plt.legend([exp_name], loc='upper right')
    # plt.savefig('grafico.png', dpi=300)
    plt.show()

    plt.title('Milhões de Instruções por Segundo (MIPS)')
    plt.xlabel('Tamanho do BPB')
    plt.ylabel('MPIS')
    plt.bar(values_x,values_mips)
    plt.legend([exp_name], loc='upper right')
    # plt.savefig('grafico.png', dpi=300)
    plt.show()

    plt.title('Tempo de CPU')
    plt.xlabel('Tamanho do BPB')
    plt.ylabel('ns')
    plt.bar(values_x,values_tcpu)
    plt.legend([exp_name], loc='upper right')
    # plt.savefig('grafico.png', dpi=300)
    plt.show()

    plt.title('Taxa de acerto')
    plt.xlabel('Tamanho do BPB')
    plt.ylabel('%')
    plt.bar(values_x,values_acc)
    plt.legend([exp_name], loc='upper right')
    # plt.savefig('grafico.png', dpi=300)
    plt.show()











