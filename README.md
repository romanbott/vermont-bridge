# Vermont Bridge Deadlock Simulation

A C-based simulation demonstrating the classic "Vermont Bridge" concurrency problem, highlighting how race conditions lead to deadlocks and how to resolve them using mutexes and semaphores.

## The Problem

A single-lane bridge connects the two Vermont villages of North Tunbridge and South Tunbridge. Farmers (modeled as threads) use this bridge to deliver their produce. 

Because Vermont farmers are stubborn, they cannot back up. The bridge is modeled as two distinct segments: a **South Entrance** and a **North Entrance**. If a Northbound driver locks the South Entrance at the exact same time a Southbound driver locks the North Entrance, neither can move forward. This circular wait creates a **deadlock**.

## Compilation


```bash
gcc bridge.c -o bridge -lpthread
```

## Usage
Run with the following flags:
- `--total_drivers`: number of driver threads to be spawned. It should be an integer greater than zero.
- `--semaphore`: Optional flag. If passed a semaphore will be used and no deadlocks will occur.
