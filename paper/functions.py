import pandas as pd 
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from itertools import cycle
from collections import OrderedDict


def plot_substances_properties_vs_temperature(results_csv_file, substances, lables, pressure=0, property=''):

    # plot settings
    mpl.rcParams['lines.linewidth']=2
    mpl.rcParams['axes.labelsize']=20
    mpl.rcParams['axes.linewidth']=2
    mpl.rcParams['font.size']=18
    mpl.rcParams['figure.figsize']=[9,9]
    mpl.rcParams['figure.subplot.left'] = 0.2
    

#    fig = plt.figure()

    plt.rc('grid', linestyle="--", color='gray')
    plt.grid(True)
    markers = [m for m in Line2D.markers]
    data = pd.read_csv(results_csv_file)
    if pressure != 0.0:
    	data = data.loc[data.iloc[:, 2] == pressure]
    c_cycle = cycle(plt.rcParams['axes.prop_cycle'].by_key()['color'])
    for s, substance in enumerate(substances):
        data_select = data.loc[data['Symbol']==substance]
        c = next(c_cycle)
        m_cycle = cycle(markers)
        next(m_cycle) 
        next(m_cycle)
        for column in data_select.columns[3:]: # loop over properties
            plt.plot(data_select.iloc[:, 1], data_select[column], color=c, 
                     marker=next(m_cycle), label=lables[s], markersize=12, markeredgecolor="w")
    handles, labels = plt.gca().get_legend_handles_labels()
    by_label = OrderedDict(zip(labels, handles))
    #legend1 = plt.legend(by_label.values(), by_label.keys(), loc=9, bbox_to_anchor=(1.15, 1)) #, loc=legend_loc)
    plt.legend() #, loc=legend_loc)
        
    #plt.gca().add_artist(legend1)
    plt.xlabel("Temperature [$^\circ$C]")
    plt.ylabel(property)

    return plt

def plot_properties_vs_temperature(results_csv_file, substances, pressure=0):

# plot settings
    mpl.rcParams['lines.linewidth']=2
    mpl.rcParams['axes.labelsize']=20
    mpl.rcParams['axes.linewidth']=2
    mpl.rcParams['font.size']=18
    mpl.rcParams['figure.figsize']=[9,9]

#    fig = plt.figure()

    plt.rc('grid', linestyle="--", color='gray')
    plt.grid(True)
    markers = [m for m in Line2D.markers]
    data = pd.read_csv(results_csv_file)
    if pressure != 0.0:
    	data = data.loc[data.iloc[:, 2] == pressure]
    c_cycle = cycle(plt.rcParams['axes.prop_cycle'].by_key()['color'])
    for s, substance in enumerate(substances):
        data_select = data.loc[data['Symbol']==substance]
        c = next(c_cycle)
        m_cycle = cycle(markers)
        next(m_cycle) 
        next(m_cycle)
        for column in data_select.columns[3:]: # loop over properties
            plt.plot(data_select.iloc[:, 1], data_select[column], color=c, 
                     marker=next(m_cycle), label=column + ' '+ substance, markersize=12, markeredgecolor="w")
    handles, labels = plt.gca().get_legend_handles_labels()
    by_label = OrderedDict(zip(labels, handles))
    legend1 = plt.legend(by_label.values(), by_label.keys(), loc=9, bbox_to_anchor=(1.15, 1)) #, loc=legend_loc)
    lines = plt.gca().get_lines()
        
    plt.gca().add_artist(legend1)
    plt.xlabel("Temperature ($^\circ$C)")
    plt.ylabel("ThermoProp")

    return plt

           
