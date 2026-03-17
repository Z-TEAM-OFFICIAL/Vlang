# simplethread.v – Simplified threading model for easy parallel tasks
# This module provides a set of easy‑to‑use constants for basic concurrency.

# Number of worker threads (recommended)
val WORKER_THREADS = 2

# Task status
val TASK_PENDING = 0
val TASK_RUNNING = 1
val TASK_COMPLETE = 2
val TASK_FAILED = 3

# Simple task queue size
val TASK_QUEUE_SIZE = 16

# Example of a parallel loop (conceptual – unroll at compile time)
# if WORKER_THREADS > 1
#     val chunk_size = 10
# else
#     val chunk_size = 40
# end

# Use these constants to decide how to split work in your game.
# For instance, you could compile different code paths depending
# on whether you want simple single‑threaded or parallel behaviour.
