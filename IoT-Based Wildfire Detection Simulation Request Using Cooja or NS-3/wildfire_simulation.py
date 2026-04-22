
import random

def run_simulation(num_nodes, area_size, sensing_interval, battery_capacity, simulation_duration):
    # Simplified model for demonstration purposes

    # 1. Detection Latency (time to detect fire event)
    # Assume a fire event occurs randomly within the simulation duration
    # And detection depends on sensing interval and network size
    avg_detection_latency = sensing_interval * (num_nodes / 100) * random.uniform(0.8, 1.2)

    # 2. Energy Consumption (per node + network average)
    # Energy consumption depends on sensing interval, transmission, and node activity
    # Assume a base consumption per node per second
    base_energy_consumption_per_second = 0.001 # mAh/second
    energy_per_node = base_energy_consumption_per_second * simulation_duration * random.uniform(0.9, 1.1)
    network_average_energy = energy_per_node # Simplified, as all nodes are similar

    # 3. Packet Delivery Ratio (PDR)
    # PDR depends on network density and reliability
    pdr = 1.0 - (num_nodes / 150) * random.uniform(0.05, 0.15) # Higher nodes, slightly lower PDR
    if pdr < 0.7: pdr = 0.7 # Minimum PDR

    # 4. False Positive / True Positive Rates (simplified)
    # Assume a fixed true positive rate and a small false positive rate
    true_positive_rate = 0.95
    false_positive_rate = 0.05

    return {
        "detection_latency": avg_detection_latency,
        "energy_per_node": energy_per_node,
        "network_average_energy": network_average_energy,
        "pdr": pdr,
        "true_positive_rate": true_positive_rate,
        "false_positive_rate": false_positive_rate
    }

if __name__ == "__main__":
    # Simulation parameters from the request
    simulation_params = {
        "num_nodes": [50, 100, 150],
        "area_size": "500m x 500m",
        "sensing_interval": 30, # seconds
        "battery_capacity": 1000, # mAh
        "simulation_duration": 1000 # seconds
    }

    results = []
    for nodes in simulation_params["num_nodes"]:
        print(f"Running simulation for {nodes} nodes...")
        sim_output = run_simulation(
            nodes,
            simulation_params["area_size"],
            simulation_params["sensing_interval"],
            simulation_params["battery_capacity"],
            simulation_params["simulation_duration"]
        )
        results.append({"num_nodes": nodes, **sim_output})

    import pandas as pd
    df = pd.DataFrame(results)
    print("\nSimulation Results:")
    print(df.to_string())

    df.to_csv("simulation_results.csv", index=False)
    print("\nResults saved to simulation_results.csv")

    # Generate a simple plot for PDR vs. Number of Nodes
    import matplotlib.pyplot as plt

    plt.figure(figsize=(10, 6))
    plt.plot(df["num_nodes"], df["pdr"], marker='o')
    plt.title("Packet Delivery Ratio vs. Number of Nodes")
    plt.xlabel("Number of Nodes")
    plt.ylabel("PDR")
    plt.grid(True)
    plt.savefig("pdr_plot.png")
    print("PDR plot saved to pdr_plot.png")

    # Generate a simple plot for Energy Consumption vs. Number of Nodes
    plt.figure(figsize=(10, 6))
    plt.plot(df["num_nodes"], df["energy_per_node"], marker='o', color='red')
    plt.title("Energy Consumption Per Node vs. Number of Nodes")
    plt.xlabel("Number of Nodes")
    plt.ylabel("Energy Consumption (mAh)")
    plt.grid(True)
    plt.savefig("energy_consumption_plot.png")
    print("Energy consumption plot saved to energy_consumption_plot.png")

    # Generate a simple plot for Detection Latency vs. Number of Nodes
    plt.figure(figsize=(10, 6))
    plt.plot(df["num_nodes"], df["detection_latency"], marker='o', color='green')
    plt.title("Detection Latency vs. Number of Nodes")
    plt.xlabel("Number of Nodes")
    plt.ylabel("Detection Latency (seconds)")
    plt.grid(True)
    plt.savefig("detection_latency_plot.png")
    print("Detection latency plot saved to detection_latency_plot.png")


