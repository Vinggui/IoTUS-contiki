import matplotlib.pyplot as plot
import matplotlib as mpl
import metrics_funcs_multiple_exp as metric
import numpy, math
import os


experiment = 'single-node-cons-check'
# chart = metric.get_energy_consumption
chart = metric.get_power_consumption
# compare_to = "std"
# compare_to = "iotus"
compare_to = "both"



# Number of experiments done
Repeated = 10

# Parameters
volt = 3.0
curr_tx = 0.0177
curr_rx = 0.020
curr_cpu = 0.0018
curr_lpm = 0.0000054
curr_idle_tx = 0
curr_idle_rx = 0.000001

# Protocol events (Mili sec)

# WAKEUP_LISTEN_DURATION = 0,438*2
# WAKEUP_PERIOD = 125,031

RADIO_ON_DUR = 0
if compare_to == "std-rx":
    #For standard
    RADIO_ON_DUR = 8
else:
    #For iotus
    # RADIO_ON_DUR = 2.202
    RADIO_ON_DUR = 1.4

# Expected value would be 1.34375ms
# RADIO_TX_DUR = 1.526
RADIO_TX_DUR = 1.34375

RADIO_TX_PERIOD = 8000


cpu_cons_period = volt*curr_cpu*RADIO_TX_PERIOD
radio_tx_cons_period = volt*curr_tx*(RADIO_TX_DUR)
radio_rx_cons_period = volt*curr_rx*(RADIO_ON_DUR-RADIO_TX_DUR)
radio_idle_cons_period = volt*curr_idle_rx*(RADIO_TX_PERIOD-RADIO_ON_DUR)

radio_cons_period = radio_tx_cons_period
radio_cons_period += radio_rx_cons_period
# radio_cons_period += radio_idle_cons_period

math_cons_exp = (radio_cons_period/(RADIO_TX_PERIOD))

#################################################################################
# process the exp data
pt_sample_rate = 2
point_rate_reduction = 8

# Points start from 0 and finals are negative, so -1 is actually the last point of the list
cut_points_value_ini = 0
cut_points_value_final = 0


mpl.rcParams["savefig.directory"] = os.chdir(os.path.dirname(__file__))

path_iotus = './'+experiment+'/iotus/'
path_std = './'+experiment+'/std/'

max_seq_number = 0
array_data_to_work_to_plot1 = []
array_data_to_work_to_plot2 = []

for rep in range(1,Repeated+1):
    # Open files
    file_to_read1 = 0
    file_to_read2 = 0
    file_to_read1 = open(path_std + 'COOJA-std-cons-check-'+str(rep)+'.txt','r')
    file_to_read2 = open(path_iotus + 'COOJA-iotus-cons-check-'+str(rep)+'.txt','r')

    # Create arrays
    data_to_work1 = metric.get_nodes_cons_array(1, file_to_read1)
    data_to_work2 = metric.get_nodes_cons_array(1, file_to_read2)


    iotus_energy_cons1 = chart(data_to_work1[0],data_to_work1[1],point_rate_reduction)
    iotus_energy_cons2 = chart(data_to_work2[0],data_to_work2[1], point_rate_reduction)


    data_to_work_to_plot1 = []
    data_to_work_to_plot2 = []

    data_to_work_to_plot1 = iotus_energy_cons1[0]
    data_to_work_to_plot2 = iotus_energy_cons2[0]

    array_data_to_work_to_plot1.append(data_to_work_to_plot1)
    array_data_to_work_to_plot2.append(data_to_work_to_plot2)

    max_seq_number = data_to_work1[1]

if Repeated > 1:
    mean_data_to_work_to_plot1 = numpy.mean(array_data_to_work_to_plot1, 0)
    mean_data_to_work_to_plot2 = numpy.mean(array_data_to_work_to_plot2,0)
else:
    mean_data_to_work_to_plot1 = array_data_to_work_to_plot1[0]
    mean_data_to_work_to_plot2 = array_data_to_work_to_plot2[0]


########################################################
#					Plot Energy
########################################################
font = {'family': 'sans',
        'color':  'darkred',
        'weight': 'normal',
        'size': 16,
        }

x_data = range(0,pt_sample_rate*(max_seq_number+7),pt_sample_rate*point_rate_reduction)
x_data = x_data[cut_points_value_ini:-cut_points_value_final-1]

#Generate the points that should be used
math_cons_points = []
for value in x_data:
    if chart == metric.get_power_consumption:
        math_cons_points.append(math_cons_exp*1000)
    else:
        math_cons_points.append(value*math_cons_exp)



if compare_to == "std" or compare_to == "std-rx" or compare_to == "both":
    plot.plot(x_data,mean_data_to_work_to_plot1[cut_points_value_ini:math.ceil(max_seq_number/point_rate_reduction)-cut_points_value_final],'b-')
if compare_to == "iotus" or compare_to == "both":
    plot.plot(x_data, mean_data_to_work_to_plot2[cut_points_value_ini:math.ceil(max_seq_number / point_rate_reduction)-cut_points_value_final], 'r-')
plot.xlabel('Time (sec)')
plot.plot(x_data,math_cons_points,'g-')

legend = []
if compare_to == "std" or compare_to == "std-rx" or compare_to == "both":
    legend.append('Standard stack')
if compare_to == "iotus" or compare_to == "both":
    legend.append('IoTUS stack')
legend.append('Math expectation')

plot.legend(legend)

if chart == metric.get_energy_consumption:
    plot.ylabel('Energy consumption (mJ)')
    #plot.xlim(0, 7)
    # plot.ylim(0, 1.4)
elif chart == metric.get_power_consumption:
    plot.ylabel('Power consumption ($\mu$W)')

    print("Mean consumption for stadard was: " + str(numpy.mean(mean_data_to_work_to_plot1)))
    print("Mean consumption for iotus was: "+str(numpy.mean(mean_data_to_work_to_plot2)))
    print("Mean consumption for math expected was: " + str(numpy.mean(math_cons_points)))
    plot.text(200, 0.003, 'Math mean:', fontdict=font)
    plot.text(200, 0.002, 'New stack mean:', fontdict=font)
    plot.text(200, 0.001, 'Standard stack mean:', fontdict=font)

    plot.text(1200, 0.003, "{:.4}".format(numpy.mean(math_cons_points)), fontdict=font)
    plot.text(1200, 0.002, "{:.4}".format(numpy.mean(mean_data_to_work_to_plot2)), fontdict=font)
    plot.text(1200, 0.001, "{:.4}".format(numpy.mean(mean_data_to_work_to_plot1)), fontdict=font)
    plot.ylim(0, 0.021)
else:
    plot.ylabel('Energy consumption (mJ)')
    # plot.xlim(0, 7)
    # plot.ylim(0, 1.4)
plot.show()


