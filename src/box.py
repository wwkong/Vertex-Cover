import os, sys, getopt
import csv
import math
import matplotlib.pyplot as plt
import numpy as np

def readfile(tfile_path, c):
    f = open(tfile_path)
    csv_f = csv.reader(f)
    for time, quality in csv_f:
        if int(quality) == c:
            return time
    return 0

def generate_csv(tracefile, optimal,quality=[1,2,3,4,5]):
    result = {}
    run = 0
    for tfile in os.listdir(tracefile):
        if tfile.endswith(".trace"):
            run += 1
            result[run] = {}
            tfile_path = tracefile + os.path.sep + tfile
            print(tfile_path)
            for q in quality:
                c = optimal + (optimal * q/100.0)
                c = int(c)
                #print(c)
                time = readfile(tfile_path, c)
                print(time)
                result[run][q] = time
    return result

def write_result(tracefile, results, quality):
    inputdir = os.path.basename(tracefile)
    soln_output = open(os.getcwd() + os.path.sep + inputdir + "boxplot.csv", 'w')

    soln_output.write("X0.1,X0.2,X0.3,X0.4,X0.5\n")
    for run, qdict in results.iteritems():
        for q in quality:
            if q > quality[0]:
                soln_output.write(',')
            print(qdict.get(q))
            soln_output.write(qdict.get(q))
        soln_output.write("\n")
    soln_output.close()

def main(argv):
    try:
        opts, args = getopt.getopt(argv,"hf:o:")
    except getopt.GetoptError:
        print "box.py -f <tracefile> -o <optimal> -t <max time> -q <max quality>"
        sys.exit(2)
    for opt, arg in opts:
        if opt == "-h":
            print "box.py -f <tracefile> -o <optimal>"
            sys.exit()
        elif opt == '-f':
            print "tracefile: {}".format(arg)
            tracefile = arg
        elif opt == '-o':
            print "optimal: {}".format(arg)
            optimal = float(arg)
    quality=[1,2,3,4,5]
    result = generate_csv(tracefile, optimal, quality)
    write_result(tracefile, result, quality)
    print("I am done")

if __name__ == "__main__":
    main(sys.argv[1:])
    