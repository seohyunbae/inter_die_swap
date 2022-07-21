import matplotlib.pyplot as plt
import matplotlib.patches as patches
import os
from tqdm import tqdm


def parse_input(filedir="../test/case1.out"):
    data = {}
    data_top = {}
    data_bot = {}
    die_dim = []
    with open(filedir, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("DIEDIM"):
                line = line.split((" "))
                x_ax = [int(line[1]), int(line[2])]
                y_ax = [int(line[3]), int(line[4])]
                die_dim.append(x_ax)
                die_dim.append(y_ax)
            elif line.startswith("DRAW"):
                line = line.split(" ")
                epoch = int(line[1])
                if epoch not in data.keys():
                    data[epoch] = []
                if epoch not in data_top.keys():
                    data_top[epoch] = []
                if epoch not in data_bot.keys():
                    data_bot[epoch] = []
                pmin = (float(line[2]), float(line[3]))
                pmax = (float(line[4]), float(line[5]))
                rect = (pmin, pmax)
                data[epoch].append(rect)
                if line[6].startswith("TO"):
                    data_top[epoch].append(rect)
                elif line[6].startswith("BO"):
                    data_bot[epoch].append(rect)
    return die_dim, data, data_bot, data_top


def create_plot(pltdir="./case1", filedir="../test/case1.out"):
    die_dim, data, data_bot, data_top = parse_input(filedir=filedir)
    row_max = (die_dim[0][1] - die_dim[0][0])
    col_max = (die_dim[1][1] - die_dim[1][0])
    row_scale = col_scale = 1
    while row_max * 100 > 2048:
        row_max /= 2
        row_scale *= 2
    while col_max * 100 > 2048:
        col_max /= 2
        col_scale *= 2

    if not os.path.isdir(pltdir):
        os.mkdir(pltdir)
    print("BOTH DIE")
    for epoch in data.keys():
        fig = plt.figure(figsize=(row_max, col_max))
        plt.plot(die_dim[0], die_dim[1], color='None')
        name = str(epoch) + ".png"
        path = os.path.join(pltdir, name)
        # ax = plt.subplots()
        for rect in tqdm(data[epoch]):
            plt.plot(rect[0][0], rect[0][1], marker="o", markersize=10)
        plt.savefig(path)
        plt.close(fig)
    print("BOT DIE")
    for epoch in data_bot.keys():
        fig = plt.figure(figsize=(row_max, col_max))
        plt.plot(die_dim[0], die_dim[1], color='None')
        name = str(epoch) + "_BOT.png"
        path = os.path.join(pltdir, name)
        for rect in tqdm(data_bot[epoch]):
            plt.plot(rect[0][0], rect[0][1], marker="o", markersize=10)
        plt.savefig(path)
        plt.close(fig)
    print("TOP DIE")
    for epoch in data_top.keys():
        fig = plt.figure(figsize=(row_max, col_max))
        plt.plot(die_dim[0], die_dim[1], color='None')
        name = str(epoch) + "_TOP.png"
        path = os.path.join(pltdir, name)
        for rect in tqdm(data_top[epoch]):
            plt.plot(rect[0][0], rect[0][1], marker="o", markersize=10)
        plt.savefig(path)
        plt.close(fig)

#
# if __name__ == '__main__':
#     pltname = "./case2"
#     filedir = "../test/case2.out"
#     create_fig(pltname, filedir)