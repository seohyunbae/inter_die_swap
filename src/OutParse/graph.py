import argparse
import matplotlib.pyplot as plt
import sys


def parse_input(filedir="../test/case1.out"):
    data = []
    index = 0
    with open(filedir, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("EPOCH"):
                line = line.split(" ")
                # for word in line:
                #     print(word)
                # num_epoch = int(line[1])
                HPWL = float(line[4])
                # data[index] = (num_epoch, HPWL)
                data.append(HPWL)
    return data


def create_graph(pltname="./case1.jpg", filedir="../test/case1.out"):
    data = parse_input(filedir=filedir)
    plt.plot(data)
    plt.savefig(pltname)
    plt.close('all')


def create_summary(output_dir="../result/1/summary.txt", filedir="../result/1/out/case1.out"):
    summary = {}
    with open(filedir, 'r') as f:
        lines = f.readlines()
        for line in lines:
            if line.startswith("DB"):
                line = line.strip('\n')
                line = line.split(" ")
                summary["DB"] = line[2]
            elif line.startswith("FM"):
                line = line.strip('\n')
                line = line.split(" ")
                summary["FM"] = line[2]
            elif line.startswith("IP"):
                line = line.strip('\n')
                line = line.split(" ")
                summary["IP"] = line[2]
            elif line.startswith("GP"):
                line = line.strip('\n')
                line = line.split(" ")
                summary["GP"] = line[2]
            elif line.startswith("Initial"):
                line = line.strip('\n')
                line = line.split(" ")
                summary["Initial HPWL"] = line[3]
            elif line.startswith("INFO"):
                line = line.strip('\n')
                line = line.split(" ")
                if len(line) > 10:
                    summary["IP HPWL"] = line[10]
            elif line.startswith("EPOCH"):
                line = line.strip('\n')
                line = line.split(" ")
                summary["Final HPWL"] = line[4]
        f.close()
    old_stdout = sys.stdout
    sys.stdout = open(output_dir, 'w')
    print("HPWL @ Center : ", summary["Initial HPWL"])
    print("HPWL after IP : ", summary["IP HPWL"][:-1])
    print("HPWL after GP : ", summary["Final HPWL"])
    print("DB runtime : ", summary["DB"])
    print("FM runtime : ", summary["FM"])
    print("IP runtime : ", summary["IP"])
    print("GP runtime : ", summary["GP"])
    sys.stdout = old_stdout

# if __name__ == '__main__':
#     pltname = "./case2.jpg"
#     filedir = "../test/case2.out"
#     create_fig(pltname, filedir)
