import matplotlib.pyplot as plt
import shlex, subprocess

def drawPlot(name):
    x = []
    y = []
    with open('update.' + name + '.txt') as file:
        for line in file:
            tmp = line.split()
            x.append(float(tmp[0]))
            #y.append(1000 * float(tmp[1]) / float(tmp[0]))
            y.append(float(tmp[2]))
    plt.plot(x, y, label = name)

#plt.axis([0, 100000, 0, 0.3])


tests = ["OopWithPools", "Oop", "mustache", "mustache-lua", "EnTT", "OpenEcs", "EntityX"]
entity_counts = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 6000, 7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000, 100000, 250000, 500000, 750000, 1000000]#, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000, 8000000, 9000000, 10000000]

#total = 0
def runTests():
    #global total
    #for i in range(1, 100000000):
        #total += i
    for test in tests:
        for count in entity_counts:
            print("Run " + test + " with " + str(count) + " entities")
            if test == "mustache-lua":
                subprocess.run(["luajit", "api_test.lua", test, str(count)])
            else:
                subprocess.run(["./EcsBenchmark", test, str(count), "true", "true"])

def showResults():
    for test in tests:
        try:
            drawPlot(test)
        except FileNotFoundError:
            print("Can not open file for: " + test)
    plt.xlabel('Entity count')
    plt.ylabel('Update time(ms)')
    plt.grid(True)
    plt.legend()
    plt.show()


runTests()
showResults()

#drawPlot('OOP')
#drawPlot('EntityX')
#drawPlot('EnTT')
#drawPlot('mustache')


#plt.xlabel('Entity count')
#plt.ylabel('Update time(ms)')
#plt.show()
