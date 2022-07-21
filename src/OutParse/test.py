from graph import create_graph
from plot_instance import create_plot
from graph import create_summary
from config_gen import *
import argparse
import json
import os


config_file = "./config.json"
case_file_root = "./../case"
result_path = "../result"
out_path_name = "out"
plot_path_name = "plot"
graph_path_name = "graph"
summary_path_name = "summary"


def get_config(config_num="1"):
    with open(config_file) as f:
        config_dict = json.load(f)
        config_set = config_dict[config_num]
        ns_epoch = int(config_set["ns_epoch"])
        lr = float(config_set["lr"])
        lr_decay = float(config_set["lr_decay"])
        momentum_x = float(config_set["momentum_X"])
        momentum_y = float(config_set["momentum_Y"])
        charge_weight = float(config_set["charge_weight"])
        binX = int(config_set["binX"])
        binY = int(config_set["binY"])
    return ns_epoch, lr, lr_decay, momentum_x, momentum_y, charge_weight, binX, binY


def execute_binary_file(case="all", config_name="1", verbose=True, mode="plot"):
    if not os.path.isdir(result_path):
        os.mkdir(result_path)
    out_path = os.path.join(result_path, config_name)
    if not os.path.isdir(out_path):
        os.mkdir(out_path)
    out_file_path = os.path.join(out_path, out_path_name)
    if not os.path.isdir(out_file_path):
        os.mkdir(out_file_path)
    plot_path = os.path.join(out_path, plot_path_name)
    if not os.path.isdir(plot_path):
        os.mkdir(plot_path)
    graph_path = os.path.join(out_path, graph_path_name)
    if not os.path.isdir(graph_path):
        os.mkdir(graph_path)
    summary_path = os.path.join(out_path, summary_path_name)
    if not os.path.isdir(summary_path):
        os.mkdir(summary_path)

    graph1 = os.path.join(graph_path, "case1.png")
    graph2 = os.path.join(graph_path, "case2.png")
    graph3 = os.path.join(graph_path, "case3.png")
    graph4 = os.path.join(graph_path, "case4.png")

    plot1_dir = os.path.join(plot_path, "case1")
    plot2_dir = os.path.join(plot_path, "case2")
    plot3_dir = os.path.join(plot_path, "case3")
    plot4_dir = os.path.join(plot_path, "case4")

    out1 = os.path.join(out_file_path, "case1.out")
    out2 = os.path.join(out_file_path, "case2.out")
    out3 = os.path.join(out_file_path, "case3.out")
    out4 = os.path.join(out_file_path, "case4.out")

    summary1 = os.path.join(summary_path, "case1.txt")
    summary2 = os.path.join(summary_path, "case2.txt")
    summary3 = os.path.join(summary_path, "case3.txt")
    summary4 = os.path.join(summary_path, "case4.txt")

    ns_epoch, lr, lr_decay, momentum_x, momentum_y, charge_weight, bin_x, bin_y = get_config(config_name)
    command = " " + str(ns_epoch)
    command = command + " " + str(lr) + " " + str(lr_decay)
    command = command + " " + str(momentum_x) + " " + str(momentum_y)
    command = command + " " + str(charge_weight) + " " + str(bin_x) + " " + str(bin_y)
    cmd1 = "./../main " + case_file_root + "1.txt " + command
    cmd2 = "./../main " + case_file_root + "2.txt " + command
    cmd3 = "./../main " + case_file_root + "3.txt " + command
    cmd4 = "./../main " + case_file_root + "4.txt " + command
    if verbose:
        cmd1 = cmd1 + " | tee " + out1
        cmd2 = cmd2 + " | tee " + out2
        cmd3 = cmd3 + " | tee " + out3
        cmd4 = cmd4 + " | tee " + out4
    else:
        cmd1 = cmd1 + " > " + out1
        cmd2 = cmd2 + " > " + out2
        cmd3 = cmd3 + " > " + out3
        cmd4 = cmd4 + " > " + out4

    if case == "all":
        print("================", case, config_name, "================")
        print(cmd1)
        os.system(cmd1)
        if mode == "plot":
            create_summary(output_dir=summary1, filedir=out1)
            create_graph(pltname=graph1, filedir=out1)
            if not os.path.isdir(plot1_dir):
                os.mkdir(plot1_dir)
            create_plot(pltdir=plot1_dir, filedir=out1)

        print(cmd2)
        os.system(cmd2)
        if mode == "plot":
            create_summary(output_dir=summary2, filedir=out2)
            create_graph(pltname=graph2, filedir=out2)
            if not os.path.isdir(plot2_dir):
                os.mkdir(plot2_dir)
            create_plot(pltdir=plot2_dir, filedir=out2)

        print(cmd3)
        os.system(cmd3)
        if mode == "plot":
            create_summary(output_dir=summary3, filedir=out3)
            create_graph(pltname=graph3, filedir=out3)
            if not os.path.isdir(plot3_dir):
                os.mkdir(plot3_dir)
            create_plot(pltdir=plot3_dir, filedir=out3)

        print(cmd4)
        os.system(cmd4)
        if mode == "plot":
            create_summary(output_dir=summary4, filedir=out4)
            create_graph(pltname=graph4, filedir=out4)
            if not os.path.isdir(plot4_dir):
                os.mkdir(plot4_dir)
            create_plot(pltdir=plot4_dir, filedir=out4)

    elif case == "1":
        print(cmd1)
        os.system(cmd1)
        if mode == "plot":
            create_summary(output_dir=summary1, filedir=out1)
            create_graph(pltname=graph1, filedir=out1)
            if not os.path.isdir(plot1_dir):
                os.mkdir(plot1_dir)
            create_plot(pltdir=plot1_dir, filedir=out1)
    elif case == "2":
        print(cmd2)
        os.system(cmd2)
        if mode == "plot":
            create_summary(output_dir=summary2, filedir=out2)
            create_graph(pltname=graph2, filedir=out2)
            if not os.path.isdir(plot2_dir):
                os.mkdir(plot2_dir)
            create_plot(pltdir=plot2_dir, filedir=out2)
    elif case == "3":
        print(cmd3)
        os.system(cmd3)
        if mode == "plot":
            create_summary(output_dir=summary3, filedir=out3)
            create_graph(pltname=graph3, filedir=out3)
            if not os.path.isdir(plot3_dir):
                os.mkdir(plot3_dir)
            create_plot(pltdir=plot3_dir, filedir=out3)
    elif case == "4":
        print(cmd4)
        os.system(cmd4)
        if mode == "plot":
            create_summary(output_dir=summary4, filedir=out4)
            create_graph(pltname=graph4, filedir=out4)
            if not os.path.isdir(plot4_dir):
                os.mkdir(plot4_dir)
            create_plot(pltdir=plot4_dir, filedir=out4)
    else:
        raise IOError


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--index', required=True, help='Training config 1, 2, or ...')
    parser.add_argument('-c', '--case', required=True, help='Testing case [1, 2, 3, 4, all]')
    parser.add_argument('-v', '--verbose', required=False)
    parser.add_argument('-m', '--mode', required=False)
    args = parser.parse_args()
    if args.verbose is None:
        args.verbose = False
    else:
        if args.verbose == "True" or args.verbose == "true":
            args.verbose = True
        else:
            args.verbose = False

    if args.mode == "Pipe" or args.mode == "pipe":
        args.mode = "pipe"
    elif args.mode == "Plot" or args.mode == "plot":
        args.mode = "plot"
    else:
        args.mode = "plot"
    execute_binary_file(args.case, args.index, args.verbose, args.mode)
