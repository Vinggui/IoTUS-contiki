import matplotlib.pyplot as plot
import matplotlib as mpl
import metrics_funcs_multiple_exp as metric
import numpy, math
import os

# Exp working: Broadcast, star on and off,

# experiment = 'keepAlive-30min-20b-txOnly'
# experiment = 'keepAlive-30min-20b-8stx-featureON'
experiment = 'keepAlive-30min-20b-8stx-featureOFF'
# experiment = 'keepAlive-30min-20b-8stx-featureON-OFF'
# experiment = 'broadcast-30min-20b'
# experiment = 'star-keepAlive-30min-20b-8sec-featureOFF'
# experiment = 'star-keepAlive-30min-20b-8sec-featureON'
# experiment = 'star-keepAlive-30min-20b-8sec-featureON_OFF'
# experiment = 'keepAlive-1h-20b-8stx-featureON'
max_time_axis_x = 1800

# Calculate the difference
get_difference_percent = 1
# get_difference_percent = 0

chart = metric.get_energy_consumption
# chart = metric.get_power_consumption


# Number of experiments done
Repeated = 10


IGNORE_NODE_ID = 0


# Path to the folder of IoTUS & std
path_iotus = './'+experiment+'/iotus/'
path_std = './'+experiment+'/std/'


# Number of nodes used
nodes_num = 8

# Metric out of a specific node (node number) or mean of all network (node #0)
node_to_plot = 0

#################################################################################
pt_sample_rate = 2
point_rate_reduction = 1

# Points start from 0 and finals are negative, so -1 is actually the last point of the list
cut_points_value_ini = 0
cut_points_value_final = 0


# Get the experiment parameters for this char generator
exp_file = open('./'+experiment+'/exp_chart_parameters.txt','r')
chart_pos_parameters = []
for line in exp_file:
    number = (line.split('='))[1]
    chart_pos_parameters.append(float(number))

if os.path.dirname(__file__) == "":
    mpl.rcParams["savefig.directory"] = os.chdir(os.getcwd())
else:
    mpl.rcParams["savefig.directory"] = os.chdir(os.path.dirname(__file__))

max_seq_number = 0
array_iotus_data_to_plot = []
array_std_data_to_plot = []
for rep in range(1,Repeated+1):
    # Open files
    iotus_file = open(path_iotus + 'COOJA-iotus-'+str(rep)+'.txt','r')
    std_file = open(path_std + 'COOJA-std-'+str(rep)+'.txt',  'r')

    # Create arrays
    iotus_data = metric.get_nodes_cons_array(nodes_num, iotus_file)
    std_data = metric.get_nodes_cons_array(nodes_num, std_file)

    # make data similar
    for node_std in std_data[0]:
        delete = True
        for node_iotus in iotus_data[0]:
            if IGNORE_NODE_ID == node_iotus.node_id:
                print("iotus node " + str(node_iotus.node_id) + " was deleted!")
                iotus_data[0].remove(node_iotus)
                continue
            if node_iotus.node_id == node_std.node_id:
                delete = False
                break
        if delete == True:
            print("std node "+str(node_std.node_id)+" was also deleted!")
            std_data[0].remove(node_std)

    for node_std in std_data[0]:
        if IGNORE_NODE_ID == node_std.node_id:
            print("std node "+str(node_std.node_id)+" was deleted!")
            std_data[0].remove(node_std)



    iotus_energy_cons = chart(iotus_data[0],iotus_data[1],point_rate_reduction)
    std_energy_cons = chart(std_data[0], std_data[1],point_rate_reduction)


    iotus_data_to_plot = []
    std_data_to_plot = []
    if node_to_plot == 0:
        #Do the mean of the network...
        iotus_data_to_plot = numpy.mean(iotus_energy_cons,0)
        std_data_to_plot = numpy.mean(std_energy_cons,0)
    else:
        iotus_data_to_plot = iotus_energy_cons[node_to_plot-1]
        std_data_to_plot = std_energy_cons[node_to_plot-1]

    array_iotus_data_to_plot.append(iotus_data_to_plot)
    array_std_data_to_plot.append(std_data_to_plot)

    if std_data[1] > iotus_data[1]:
        max_seq_number = iotus_data[1]
    else:
        max_seq_number = std_data[1]

if Repeated > 1:
    mean_iotus_data_to_plot = numpy.mean(array_iotus_data_to_plot,0)
    mean_std_data_to_plot = numpy.mean(array_std_data_to_plot,0)
else:
    mean_iotus_data_to_plot = array_iotus_data_to_plot[0]
    mean_std_data_to_plot = array_std_data_to_plot[0]

########################################################
#					Plot Energy
########################################################
font = {'family': 'serif',
        'color':  'darkred',
        'weight': 'normal',
        'size': 16,
        }

division_rest = (pt_sample_rate*max_seq_number)%(pt_sample_rate*point_rate_reduction)
x_data = range(0,(pt_sample_rate*max_seq_number + division_rest),pt_sample_rate*point_rate_reduction)
if cut_points_value_final > 0:
    x_data = x_data[cut_points_value_ini:-cut_points_value_final-1]
last_point = math.floor(max_time_axis_x/pt_sample_rate)-1

if get_difference_percent == 1:
    diff_values = metric.get_energy_consumption_difference_percent(mean_iotus_data_to_plot[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final],mean_std_data_to_plot[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final])

    plot.plot(x_data, diff_values, 'g-')

    # plot.legend(['IoTUS stack: (Piggyback On)/(Piggyback OFF)'])
    plot.legend(['IoTUS/Standard stack'])
    plot.ylabel('Energy consumption ratio')

    mean_ratio = numpy.mean(diff_values[last_point-100:last_point])
    print("Mean ratio was: "+str(mean_ratio))
    if mean_ratio > 1:
        plot.text(chart_pos_parameters[7], 0.3, 'Last 100 samples mean: '+"{:.5}".format(mean_ratio), fontdict=font)
    else:
        plot.text(chart_pos_parameters[7], 0.3, 'Last 100 samples mean: ' + "{:.4}".format(mean_ratio), fontdict=font)
    plot.ylim(0, chart_pos_parameters[6])
    plot.xlim(0, max_time_axis_x)

elif chart == metric.get_power_consumption:
    plot.plot(x_data,mean_iotus_data_to_plot[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final],'r-')
    plot.plot(x_data,mean_std_data_to_plot[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final],'b-')

    plot.legend(['IoTUS stack','Standard stack'])
    plot.ylabel('Power consumption (mW)')

    mean_iotus_value = numpy.mean(mean_iotus_data_to_plot[0:last_point])
    mean_std_value = numpy.mean(mean_std_data_to_plot[0:last_point])

    print("Mean consumption for iotus was: "+str(mean_iotus_value))
    print("Mean consumption for standard was: " + str(mean_std_value))
    plot.xlim(0, max_time_axis_x)
    plot.ylim(0, chart_pos_parameters[5])
    plot.text(chart_pos_parameters[0], chart_pos_parameters[2], 'IoTUS mean:', fontdict=font)
    plot.text(chart_pos_parameters[0], chart_pos_parameters[3], 'Standard mean:', fontdict=font)
    plot.text(chart_pos_parameters[1], chart_pos_parameters[2], "{:.4}".format(mean_iotus_value)+' mW', fontdict=font)
    plot.text(chart_pos_parameters[1], chart_pos_parameters[3], "{:.4}".format(mean_std_value)+' mW', fontdict=font)

else:
    plot.plot(x_data,mean_iotus_data_to_plot[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final],'r-')
    plot.plot(x_data,mean_std_data_to_plot[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final],'b-')

    # plot.legend(['IoTUS stack: Piggyback On','IoTUS stack: Piggyback Off'])
    plot.legend(['IoTUS stack','Standard stack'])
    plot.ylabel('Energy consumption (mJ)')
    plot.xlim(0, max_time_axis_x)
    plot.ylim(0, chart_pos_parameters[4])

plot.xlabel('Time (sec)')
plot.show()