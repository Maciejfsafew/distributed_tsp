# Parallel implementation of Travelling Salesman Problem

# Architecture
Master - Manages K-first layers of tree. Creates list of tasks, updates best solution. Assures load balancing.
Worker - Asks for tasks in asynchronous manner, delivers results.

Optimization - Pruning.

# Speedup
![Alt text](/results/Speedup.png "")

# Efficiency
![Alt text](/results/Efficiency.png "")

# Load Balancing Time
![Alt text](/results/Load_balancing.png "")

# Load Balancing Tasks
![Alt text](/results/Load_balancing_tasks.png "")
