
class TestParam:
    def __init__(self, ns_epoch, lr, lr_decay, momentum_x, momentum_y, charge_weight, binX, binY):
        self.ns_epoch = ns_epoch
        self.lr = lr
        self.lr_decay = lr_decay
        self.momentum_x = momentum_x
        self.momentum_y = momentum_y
        self.charge_weight = charge_weight
        self.binX = binX
        self.binY = binY

    def __str__(self):
        line = "    \"ns_epoch\": " + str(self.ns_epoch) + ",\n"
        line += "    \"lr\": " + str(self.lr) + ",\n"
        line += "    \"lr_decay\": " + str(self.lr_decay) + ",\n"
        line += "    \"momentum_X\": " + str(self.momentum_x) + ",\n"
        line += "    \"momentum_Y\": " + str(self.momentum_y) + ",\n"
        line += "    \"charge_weight\": " + str(self.charge_weight) + ",\n"
        line += "    \"binX\": " + str(self.binX) + ",\n"
        line += "    \"binY\": " + str(self.binY)
        return line


if __name__ == "__main__":
    numCase = 300
    numEpoch = 300
    lr_min = 0.005
    lr_step = 0.001
    lr_decay = 0.995
    momentumX = 0.01
    momentumY = 0.01
    binX = 100
    binY = 100
    binStep = 25
    print("{")
    for i in range(numCase):
        j = i % 50
        if i % 20 < 10:
            charge_weight = 0.000000001 * (10**(j//10))
        else:
            charge_weight = 0.000000005 * (10**(j//10))
        curlr = lr_min
        if i % 10 == 0:
            curlr = 0.005
        elif i % 10 == 1:
            curlr = 0.006
        elif i % 10 == 2:
            curlr = 0.007
        elif i % 10 == 3:
            curlr = 0.008
        elif i % 10 == 4:
            curlr = 0.009
        elif i % 10 == 5:
            curlr = 0.01
        elif i % 10 == 6:
            curlr = 0.011
        elif i % 10 == 7:
            curlr = 0.012
        elif i % 10 == 8:
            curlr = 0.013
        elif i % 10 == 9:
            curlr = 0.014
        else:
            raise IOError
        param = TestParam(ns_epoch=numEpoch,
                          lr=curlr,
                          lr_decay=lr_decay,
                          momentum_x=momentumX,
                          momentum_y=momentumY,
                          charge_weight=charge_weight,
                          binX=binX+binStep*(i//50),
                          binY=binY+binStep*(i//50))
        key = "  \"" + str(i) + "\":{"
        # print("  \"", str(i), "\":{")
        print(key)
        print(param)
        if i == numCase - 1:
            print("  }")
        else:
            print("  }, ")
    print("}")
