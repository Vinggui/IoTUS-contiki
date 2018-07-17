import numpy

ticks_per_sec = 32768.0
volt = 3.0
curr_tx = 0.0177
curr_rx = 0.020
curr_cpu = 0.0018
curr_lpm = 0.0000054
curr_idle_tx = 0
curr_idle_rx = 0.000001
pt_sample_rate = 2

class consumptions:
    def __init__(self):
        self.node_id = 0
        self.all_cpu = []
        self.all_lpm = []
        self.all_tx = []
        self.all_rx = []
        self.all_idle_tx = []
        self.all_idle_rx = []
        self.cpu = []
        self.lpm = []
        self.tx = []
        self.rx = []
        self.idle_tx = []
        self.idle_rx = []
        self.sqn = []


def get_nodes_cons_array(nodes_num, file):
    nodes_cons = []
    nodes_seq_num = []

    for x in range(0,nodes_num):
        cons = consumptions()
        cons.node_id = x +1
        nodes_cons.append(cons)
        nodes_seq_num.append(0)


    for line in file:
        phrase = line.split(":")
        if len(phrase) == 3:
            if phrase[2].startswith("PT "):
                # print("Frase foi: "+line)
                node_id = int(phrase[1])-1
                cons_values = phrase[2].split()
                seq_num = int(cons_values[3])

                nodes_seq_num[node_id] = seq_num
                #for seq in nodes_seq_num:
                #    if seq_num > (seq+1):
                #        print ("Sequence num error!\n")
                #        exit()
                #

                nodes_cons[node_id].all_cpu.append(int(cons_values[4]))
                nodes_cons[node_id].all_lpm.append(int(cons_values[5]))
                nodes_cons[node_id].all_tx.append(int(cons_values[6]))
                nodes_cons[node_id].all_rx.append(int(cons_values[7]))
                nodes_cons[node_id].all_idle_tx.append(int(cons_values[8]))
                nodes_cons[node_id].all_idle_rx.append(int(cons_values[9]))

                nodes_cons[node_id].cpu.append(int(cons_values[10]))
                nodes_cons[node_id].lpm.append(int(cons_values[11]))
                nodes_cons[node_id].tx.append(int(cons_values[12]))
                nodes_cons[node_id].rx.append(int(cons_values[13]))
                nodes_cons[node_id].idle_tx.append(int(cons_values[14]))
                nodes_cons[node_id].idle_rx.append(int(cons_values[15]))

                nodes_cons[node_id].sqn.append(int(cons_values[3]))


    # max_size = 0
    # for y in range(0,len(nodes_cons[0].cpu)):
    #     try:
    #         #print("saadsad  "+str(y))
    #         #print("ISSSO   "+str(nodes_cons[0].sqn[y]) +" %% "+str(nodes_cons[0].sqn[y])+" %% "+str(nodes_cons[1].sqn[y])+" %% "+str(nodes_cons[2].sqn[y])+" %% "+str(nodes_cons[3].sqn[y])+" %% "+str(nodes_cons[4].sqn[y])+" %% "+str(nodes_cons[5].sqn[y])+" %% "+str(nodes_cons[6].sqn[y])+" %% "+str(nodes_cons[7].sqn[y]))
    #         for k in nodes_cons:
    #             if nodes_cons[0].sqn[y] != k.sqn[y]:
    #                 print ("Sequence number does not match!\n")
    #                 break
    #                 #exit()
    #         if(nodes_cons[0].sqn[-1] != k.sqn[-1]):
    #             print ("Max sequence number does not match!\n")
    #             break
    #             #exit()
    #         max_size = y
    #     except Exception:
    #         print("Annoying bug....")
    #         exit()

    max_size = 0
    for node in nodes_cons:
        if node.sqn[-1] > max_size:
            max_size = node.sqn[-1]

    #exclude bad nodes
    for node in nodes_cons:
        # print (str(node.node_id)+" - test "+str(len(node.all_tx)))
        if node.sqn[-1] != max_size:
            print(str(max_size)+" Bad value: "+str(node.sqn[-1]))
            print("Sim "+str(file.name)+": bad node excluded "+str(node.node_id))
            nodes_cons.remove(node)
        elif len(node.all_tx) < (max_size+1):
            print(str(max_size+1)+" Bad tx value: "+str(len(node.all_tx)))
            print("Sim "+str(file.name)+": bad node excluded "+str(node.node_id))
            nodes_cons.remove(node)


    return [nodes_cons,max_size]

def get_energy_consumption(cons_array,max_sqn,point_rate_reduction):
    energy_cons = []
    nodes_num = len(cons_array)

    for x in range(0,nodes_num):
        points_reduction_counter = point_rate_reduction
        points_means = []
        energy_cons.append([])

        print(str(cons_array[x].node_id)+" - size cons "+str(len(cons_array[x].all_tx)))
        for y in range(0,max_sqn):
            consumption_radio  = cons_array[x].all_tx[y] * curr_tx
            consumption_radio += cons_array[x].all_rx[y] * curr_rx
            consumption_radio += cons_array[x].all_idle_tx[y] * curr_idle_tx
            consumption_radio += cons_array[x].all_idle_rx[y] * curr_idle_rx
            energy = (consumption_radio*volt)/ticks_per_sec

            if points_reduction_counter == 1:
                points_means.append(energy)
                energy_cons[x].append(numpy.mean(points_means))
                points_reduction_counter = point_rate_reduction
                points_means = []
            else:
                points_reduction_counter -= 1
                points_means.append(energy)
        if len(points_means) > 0:
            energy_cons[x].append(numpy.mean(points_means))

    return energy_cons

def get_power_consumption(cons_array,max_sqn,point_rate_reduction):
    energy_cons = []
    nodes_num = len(cons_array)

    for x in range(0,nodes_num):
        points_reduction_counter = point_rate_reduction
        points_means = []
        energy_cons.append([])

        for y in range(0,max_sqn):
            consumption_radio  = cons_array[x].tx[y] * curr_tx
            consumption_radio += cons_array[x].rx[y] * curr_rx
            consumption_radio += cons_array[x].idle_tx[y] * curr_idle_tx
            consumption_radio += cons_array[x].idle_rx[y] * curr_idle_rx
            energy = 1000.0*consumption_radio*volt/(ticks_per_sec*pt_sample_rate)

            if points_reduction_counter == 1:
                points_means.append(energy)
                energy_cons[x].append(numpy.mean(points_means))
                points_reduction_counter = point_rate_reduction
                points_means = []
            else:
                points_reduction_counter -= 1
                points_means.append(energy)
        if len(points_means) > 0:
            energy_cons[x].append(numpy.mean(points_means))

    return energy_cons


def get_energy_consumption_txonly(cons_array,max_sqn,point_rate_reduction):
    energy_cons = []
    nodes_num = len(cons_array)

    for x in range(0,nodes_num):
        energy_cons.append([])
        for y in range(0,max_sqn):
            consumption_radio  = cons_array[x].all_tx[y] * curr_tx
            # consumption_radio += cons_array[x].all_rx[y] * curr_rx
            # consumption_radio += cons_array[x].all_idle_tx[y] * curr_idle_tx
            # consumption_radio += cons_array[x].all_idle_rx[y] * curr_idle_rx
            energy = consumption_radio*volt/ticks_per_sec
            energy_cons[x].append(energy)
    return energy_cons


def get_energy_consumption_difference_percent(cons_iotus,cons_std):
    energy_cons = []
    max_size = len(cons_iotus)

    for x in range(0,max_size):
        energy_cons.append((cons_iotus[x])/cons_std[x])
    return energy_cons