from turtle import color
import matplotlib.pyplot as plt
import argparse
import os
import numpy as np

parser = argparse.ArgumentParser()

parser.add_argument('--experiment_folder', default='../result', type=str)

args = parser.parse_args()

exp_folder = args.experiment_folder

COLOR_PALETTE = ['#033B8F', '#8F2618']


#pega todas as pastas dentro de results
subdirs = [exp_folder+'/'+el for x in os.walk(exp_folder) for el in x[1]]

exp_names = []

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
    
        

for subdir in subdirs:
    files = [str(el) for x in os.walk(subdir) for el in x[2] if "result" in str(el)]

    exp_name = subdir.split('/')[-1]

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


            st = content.find('Tempo de CPU')
            st = content.find('-', st)+1
            end = content.find('\n', st)-2
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

    x_axis = np.arange(len(np.unique(x_labels))) # supondo que seja sempre 2 bits
    for i in range(n_bits-1):
        x_axis= np.concatenate([x_axis, [val+barWidth for val in x_axis]])
    

    plt.figure()
    if n_bits >0:
        n_clusters = len(np.unique(x_labels))-1
        if n_bits>1:
            ticks = [(x_axis[i]+x_axis[i+n_clusters*n_bits])/2 for i in range(len(np.unique(x_labels)))]
        else:
            ticks = x_axis

        plt.title(f'Teste: {exp_name}\nCiclos por instrução (CPI)', fontsize=18)
        for i, bit in enumerate(bits):
            y = []
            for el in values_sorted[bit-1]:
                y = np.array([el.cpi])
            y = y.flatten()
            plt.bar(x_axis[i*n_bits:i*n_bits+2], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, np.unique(x_labels))
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('CPI', fontsize=16)
        plt.legend(loc='upper right')
        
        plt.show()


        plt.title(f'Teste: {exp_name}\nMilhões de Instruções por Segundo (MIPS)', fontsize=18)
        for i, bit in enumerate(bits):
            y = []
            for el in values_sorted[bit-1]:
                y = np.array([el.mips])
            y = y.flatten()
            plt.bar(x_axis[i*n_bits:i*n_bits+2], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth*0.97, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, x_labels)
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('MIPS', fontsize=16)
        plt.legend(loc='upper right')
        
        plt.show()


        plt.title(f'Teste: {exp_name}\nTempo de CPU', fontsize=18)
        for i, bit in enumerate(bits):
            y = []
            for el in values_sorted[bit-1]:
                y = np.array([el.tcpu])
            y = y.flatten()
            plt.bar(x_axis[i*n_bits:i*n_bits+2], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth*0.97, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, x_labels)
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('ns', fontsize=16)
        plt.legend(loc='upper right')
        
        plt.show()


        plt.title(f'Teste: {exp_name}\nTaxa de acertos', fontsize=18)
        for i, bit in enumerate(bits):
            y = []
            for el in values_sorted[bit-1]:
                y = np.array([el.acc])
            y = y.flatten()
            plt.bar(x_axis[i*n_bits:i*n_bits+2], y, color=COLOR_PALETTE[i], edgecolor ='grey', width = barWidth*0.97, label=  '#Bits: '+str(bit))

        plt.xticks(ticks, x_labels)
        plt.xlabel('Tamanho do BPB', fontsize=16)
        plt.ylabel('%', fontsize=16)
        plt.legend(loc='upper right')
        
        plt.show()




