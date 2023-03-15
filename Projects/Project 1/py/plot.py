from matplotlib import pyplot
import subprocess
import time
import os
import random
import numpy as np
from scipy.interpolate import make_interp_spline

fixed_k = [250, 500, 750]
fixed_n = [3, 5, 7]

varying_k_ranges = [100, 200, 300, 400, 500, 600, 700, 800, 900, 1000]
varying_n_ranges = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

res_for_fixed_n = {
    "3": {
        "proctopk": [],
        "threadtopk": []
    },
    "5": {
        "proctopk": [],
        "threadtopk": []
    },
    "7": {
        "proctopk": [],
        "threadtopk": []
    }
}

res_for_fixed_k = {
    "250": {
        "proctopk": [],
        "threadtopk": []
    },
    "500": {
        "proctopk": [],
        "threadtopk": []
    },
    "750": {
        "proctopk": [],
        "threadtopk": []
    }
}

build_fresh = True
plot_fresh = False

# src_dir = "./Projects/Project 1"
src_dir = ".."
plot_dir = src_dir + "/plots"

def get_data() -> None:
    if build_fresh:
        # Build the program first with make
        p = subprocess.call(["make"], cwd=src_dir)

    for program in ["proctopk", "threadtopk"]:
        # Get data for fixed k first
        for k in fixed_k:
            for n in varying_n_ranges:
                args = ['./' + program, str(k), str("out-{}.txt".format(program)), str(n)]
                l = ["test/in{}.txt".format(i) for i in range(1, n + 1)]
                args.extend(l)

                start = time.time()

                p = subprocess.call(args, cwd=src_dir)

                total_time = time.time() - start
                res_for_fixed_k[str(k)][str(program)].append(round(total_time, 3))

        # Get data for fixed n
        for n in fixed_n:
            for k in varying_k_ranges:
                args = ['./' + program, str(k), str("out-{}.txt".format(program)), str(n)]
                l = ["test/in{}.txt".format(i) for i in range(1, n + 1)] # random.sample(range(1, 11), n)
                args.extend(l)

                start = time.time()

                p = subprocess.call(args, cwd=src_dir)

                total_time = time.time() - start
                res_for_fixed_n[str(n)][str(program)].append(round(total_time, 3))

def plot_data(output_folder, res_for_fixed_k, res_for_fixed_n) -> None:
    # If the output folder doesn't exist, create it
    os.makedirs(output_folder, exist_ok=True)

    # Plot the data for fixed k first
    for k in fixed_k:
        for j in res_for_fixed_k[str(k)]:
            for program in ["proctopk", "threadtopk"]:
                X_Y_Spline = make_interp_spline(varying_n_ranges, res_for_fixed_k[str(k)][str(program)])
                X_ = np.linspace(varying_n_ranges[0], varying_n_ranges[-1], 500)
                Y_ = X_Y_Spline(X_)
                pyplot.axes(ylim=(0,2))
                pyplot.plot(X_, Y_)
                # Put markers on the points
                pyplot.scatter(varying_n_ranges, res_for_fixed_k[str(k)][str(program)], marker='.')
                pyplot.xticks(varying_n_ranges)
                pyplot.xlabel("Number of input files (N)")
                pyplot.ylabel("Time (s)")
                pyplot.title("Time taken to run {} for K = {} and varying N".format(program, k))

                for index in range(len(varying_n_ranges)):
                    pyplot.annotate(str(res_for_fixed_k[str(k)][str(program)][index]), xy=(varying_n_ranges[index], res_for_fixed_k[str(k)][str(program)][index]))

                pyplot.savefig("{}/{}_k_{}_varying_n.png".format(output_folder, program, k))
                pyplot.clf()

    for n in fixed_n:
        for j in res_for_fixed_n[str(n)]:
            for program in ["proctopk", "threadtopk"]:
                X_Y_Spline = make_interp_spline(varying_k_ranges, res_for_fixed_n[str(n)][str(program)])
                X_ = np.linspace(varying_k_ranges[0], varying_k_ranges[-1], 500)
                Y_ = X_Y_Spline(X_)
                pyplot.axes(ylim=(0,2))
                pyplot.plot(X_, Y_)
                # Put markers on the points
                pyplot.scatter(varying_k_ranges, res_for_fixed_n[str(n)][str(program)], marker='.')
                pyplot.xticks(varying_k_ranges)
                pyplot.xlabel("Number of most occuring words (K)")
                pyplot.ylabel("Time (s)")
                pyplot.title("Time taken to run {} for N = {} and varying K".format(program, n))

                for index in range(len(varying_k_ranges)):
                    pyplot.annotate(str(res_for_fixed_n[str(n)][str(program)][index]), xy=(varying_k_ranges[index], res_for_fixed_n[str(n)][str(program)][index]))

                pyplot.savefig("{}/{}_n_{}_varying_k.png".format(output_folder, program, n))
                pyplot.clf()

if __name__ == "__main__":
    get_data()

    print("Results for fixed k -- proctopk")
    for i in fixed_k:
        print ("k = {}".format(i))
        for j in res_for_fixed_k[str(i)]["proctopk"]:
            print("    {}".format(j))

    print("\nResults for fixed k -- threadtopk")
    for i in fixed_k:
        print ("k = {}".format(i))
        for j in res_for_fixed_k[str(i)]["threadtopk"]:
            print("    {}".format(j))

    print("\nResults for fixed n -- proctopk")
    for i in fixed_n:
        print("n = {}".format(i))
        for j in res_for_fixed_n[str(i)]["proctopk"]:
            print("    {}".format(j))

    print("\nResults for fixed n -- threadtopk")
    for i in fixed_n:
        print("n = {}".format(i))
        for j in res_for_fixed_n[str(i)]["threadtopk"]:
            print("    {}".format(j))

    if plot_fresh:
        plot_data(plot_dir, res_for_fixed_k, res_for_fixed_n)
