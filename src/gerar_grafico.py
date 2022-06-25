from turtle import color
import matplotlib.pyplot as plt
import argparse
import os
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--experiment_folder', default='../experiments', type=str)

args = parser.parse_args()

base_folder = args.experiment_folder

COLOR_PALETTE = ['#033B8F', '#8F2618']


#pega todas as pastas dentro de ../experiments/
exp_names = [x[1] for x in os.walk(base_folder) ][0]


class Metrics():
    def __init__(self, bit, bpb_size) -> None:
        self.bpb_size = bpb_size
        self.bit = bit
        self.acc = []
        self.mips = []
        self.cpi = []
        self.tcpu = []

    def __str__(self) -> str:
        return "CPI: "+str(self.cpi)+"\nMIPS: "+str(self.mips)+"\nTempo de CPU:"+str(self.tcpu)+"\nACC: "+str(self.acc)


exp_names = [el+'/results'for el in exp_names]
subdirs = [os.path.join(base_folder, el) for el in exp_names]


for subdir in subdirs:
    files = [str(el) for x in os.walk(subdir) for el in x[2] if "result" in str(el)]

    exp_name = subdir.split('/')[-2]

    exp_names.append(exp_name)

    values = []

    values_x = []
    labels = []
    bits = []

    for file in files:
        st = file.find('_')+1
        end= file.find('_', st)
        bit = int(file[st:end])

        if bit not in bits:
            bits.append(bit)

        st = file.find('_', end)+1
        end= file.find('.', st)
        id = file[st:end]

        values_x.append(id)
        labels.append(f'Tamanho do BPB: {id}')

        with open(os.path.join(subdir,file), 'r') as f:
            content = f.read()

            metric = Metrics(bit, bpb_size=id)

            st = content.find('CPI')
            st = content.find('-', st)+1
            end = content.find('\n', st)
            value = float(content[st: end])

            metric.cpi.append(value)

            st = content.find('MIPS')
            st = content.find('-', st)+1
            end = content.find('\n', st)
            value = float(content[st: end])

            metric.mips.append(value)


            st = content.find('Total ciclos')
            st = content.find('-', st)+1
            end = content.find('\n', st)
            print(end)
            value = int(content[st: end])

            metric.tcpu.append(value)


            st = content.find('BPB Sucess')
            st = content.find('-', st)+1
            end = content.find('\n', st)-1
            value = float(content[st: end])

            metric.acc.append(value)

            values.append(metric)

    bits   = np.unique([m.bit for m in values])
    n_bits = len(bits)

    #a pasta está vazia
    if len(values) == 0:
        continue

    values_agreg = []
    values_sorted = []

    for i in range(2):
        values_agreg.append([el for el in values if el.bit==i+1])
        values_sorted.append(sorted(values_agreg[i], key=lambda x:x.bpb_size))


    x_labels = [v.bpb_size for val in values_sorted for v in val]
    barWidth = 0.25

    x_axis = np.arange(len(np.unique(x_labels))) # 1 ou 2 bits. Vai incrementar em módulo n_bits

    for i in range(n_bits-1):
        x_axis= np.concatenate([x_axis, [val+barWidth*(i+1) for val in x_axis]])


    plt.figure()
    if n_bits >0:
        n_clusters = len(np.unique(x_labels))
        if n_bits>1:
            ticks = [(x_axis[i]+x_axis[i+n_clusters])/2 for i in range(len(np.unique(x_labels)))]
        else:
            ticks = x_axis

        plt.title(f'Teste: {exp_name}\nCiclos por instrução (CPI)', fontsize=18)
        for i, bit in enumerate(bits):
            y = np.array([])
            for el in values_sorted[bit-1]:
                y = np.append(y, el.cpi)

            idx = list(range(i, len(x_axis), 2))

            x_arr = np.array(x_axis)
            plt.bar(x_axis[i*n_clusters:i*n_clusters+n_clusters], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, np.unique(x_labels))
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('CPI', fontsize=16)
        plt.legend(loc='best') #plt.legend(loc='upper right')

        plt.savefig(f'{base_folder}/{exp_name}/charts/CPI.png')
        # plt.show()
        plt.clf() # clear plot memory

        plt.title(f'Teste: {exp_name}\nMilhões de Instruções por Segundo (MIPS)', fontsize=18)
        for i, bit in enumerate(bits):
            y = np.array([])
            for el in values_sorted[bit-1]:
                y = np.append(y, el.mips)

            idx = list(range(i, len(x_axis), 2))

            x_arr = np.array(x_axis)
            plt.bar(x_axis[i*n_clusters:i*n_clusters+n_clusters], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, np.unique(x_labels))
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('MIPS', fontsize=16)
        plt.legend(loc='best') # plt.legend(loc='upper right')
        plt.savefig(f'{base_folder}/{exp_name}/charts/MIPS.png')
        # plt.show()
        plt.clf()


        plt.title(f'Teste: {exp_name}\nCiclos de CPU', fontsize=18)
        for i, bit in enumerate(bits):
            y = np.array([])
            for el in values_sorted[bit-1]:
                print( el.tcpu)
                y = np.append(y, el.tcpu)

            idx = list(range(i, len(x_axis), 2))

            x_arr = np.array(x_axis)
            plt.bar(x_axis[i*n_clusters:i*n_clusters+n_clusters], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, np.unique(x_labels))
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('ciclos', fontsize=16)
        plt.legend(loc='best') # plt.legend(loc='upper right')
        plt.savefig(f'{base_folder}/{exp_name}/charts/CYCLES.png')
        # plt.show()
        plt.clf()


        plt.title(f'Teste: {exp_name}\nTaxa de acertos', fontsize=18)
        for i, bit in enumerate(bits):
            y = np.array([])
            for el in values_sorted[bit-1]:
                y = np.append(y, el.acc)

            idx = list(range(i, len(x_axis), 2))

            x_arr = np.array(x_axis)
            plt.bar(x_axis[i*n_clusters:i*n_clusters+n_clusters], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, np.unique(x_labels))
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('%', fontsize=16)
        plt.legend(loc='best') # plt.legend(loc='upper right')
        plt.savefig(f'{base_folder}/{exp_name}/charts/ACC.png')
        # plt.show()
        plt.clf()
