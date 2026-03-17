#!/usr/bin/env python3
"""
generate_modules.py

Creates the modules/ directory and all V module files.
Run this script in the same folder as your vlang compiler.
"""

import os

MODULES_DIR = "modules"

# Content of each module file
files = {
    "math.c": r"""# math.c – Mathematical constants and pre‑computed values
# All values are integers; floating‑point results are scaled for precision.

# Basic constants
val PI = 3
val E = 2
val PHI = 2      # golden ratio (1.618 → 2)

# Trigonometric values (scaled by 1000 for integer use)
# sin(0) = 0
val SIN_0 = 0
# sin(30°) = 0.5 → 500
val SIN_30 = 500
# sin(45°) ≈ 0.7071 → 707
val SIN_45 = 707
# sin(60°) ≈ 0.8660 → 866
val SIN_60 = 866
# sin(90°) = 1 → 1000
val SIN_90 = 1000

# cos(0) = 1 → 1000
val COS_0 = 1000
# cos(30°) ≈ 0.8660 → 866
val COS_30 = 866
# cos(45°) ≈ 0.7071 → 707
val COS_45 = 707
# cos(60°) = 0.5 → 500
val COS_60 = 500
# cos(90°) = 0
val COS_90 = 0

# Square roots (rounded to nearest integer)
val SQRT_2 = 1    # 1.414 → 1
val SQRT_3 = 2    # 1.732 → 2
val SQRT_5 = 2    # 2.236 → 2

# Logarithms (natural, base 10) – scaled by 1000
val LN_2 = 693    # 0.693 * 1000
val LN_10 = 2303  # 2.3026 * 1000
val LOG10_2 = 301 # 0.3010 * 1000

# Use these constants in your expressions like:
# val area = PI * radius * radius
""",

    "multithreading.c": r"""# multithreading.c – Constants for multithreading concepts
# These values can be used to control parallel behaviour in your game logic.

# Number of CPU cores (adjust to your system)
val CPU_CORES = 4

# Thread priority levels
val THREAD_PRIORITY_LOWEST = 0
val THREAD_PRIORITY_BELOW_NORMAL = 1
val THREAD_PRIORITY_NORMAL = 2
val THREAD_PRIORITY_ABOVE_NORMAL = 3
val THREAD_PRIORITY_HIGHEST = 4

# Maximum number of threads allowed
val MAX_THREADS = 64

# Synchronisation objects
val MUTEX_UNLOCKED = 0
val MUTEX_LOCKED = 1

# Example usage:
# if CPU_CORES >= 4
#     set parallel_tasks = 4
# else
#     set parallel_tasks = 1
# end
""",

    "definer.v": r"""# definer.v – Common game constants and utility definitions

# Game world constants
val MAX_PLAYERS = 4
val STARTING_HEALTH = 100
val STARTING_GOLD = 50
val MAX_LEVEL = 60

# Damage values
val SWORD_DAMAGE = 10
val BOW_DAMAGE = 8
val FIREBALL_DAMAGE = 25
val HEAL_POTION = 20

# Experience points per level
val XP_PER_LEVEL = 100
val XP_BONUS = 50

# Magic constants
val MANA_PER_INTELLECT = 5
val MAX_MANA = 200

# Conditional definitions based on debug mode
if 1   # set to 0 to disable debug features
    val DEBUG_ENABLED = 1
    val DEBUG_VERBOSE = 0
else
    val DEBUG_ENABLED = 0
end

# Example: compute total score formula
val BASE_SCORE = 1000
val SCORE_MULTIPLIER = 3
val MAX_SCORE = BASE_SCORE * SCORE_MULTIPLIER + 500
""",

    "simplethread.v": r"""# simplethread.v – Simplified threading model for easy parallel tasks
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
""",

    "time.c": r"""# time.c – Time‑related constants and conversions

# Seconds in various units
val SECONDS_PER_MINUTE = 60
val MINUTES_PER_HOUR = 60
val HOURS_PER_DAY = 24
val SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR
val SECONDS_PER_DAY = SECONDS_PER_HOUR * HOURS_PER_DAY

# Frames per second (for game loops)
val FPS_TARGET = 60
val MILLISECONDS_PER_FRAME = 1000 / FPS_TARGET

# Timeout values (in seconds)
val SHORT_TIMEOUT = 5
val MEDIUM_TIMEOUT = 30
val LONG_TIMEOUT = 120

# Day/night cycle (in game minutes)
val DAY_LENGTH = 1440   # 24 hours * 60 minutes
val NIGHT_START = 1200  # 8:00 PM in minutes from midnight
val DAY_START = 360     # 6:00 AM

# Example usage in game logic:
# if current_time >= NIGHT_START or current_time < DAY_START
#     set is_night = 1
# else
#     set is_night = 0
# end
""",

    "io.c": r"""# io.c – I/O related constants for file and console operations
# These values can be used to conditionally compile different I/O strategies.

# Standard file descriptors
val STDIN  = 0
val STDOUT = 1
val STDERR = 2

# File open modes (symbolic)
val FILE_READ   = 1
val FILE_WRITE  = 2
val FILE_APPEND = 4
val FILE_BINARY = 8

# Common file paths (configurable at compile time)
val CONFIG_FILE = "config.ini"
val LOG_FILE    = "game.log"
val SAVE_FILE   = "save.dat"

# Error codes
val IO_SUCCESS = 0
val IO_ERROR_FILE_NOT_FOUND = -1
val IO_ERROR_PERMISSION     = -2
val IO_ERROR_DISK_FULL      = -3

# Buffer sizes
val IO_BUFFER_SIZE = 4096

# Example: use these to decide whether to enable logging
# if DEBUG_ENABLED
#     set log_path = LOG_FILE
# else
#     set log_path = ""
# end
""",

    "http.v": r"""# http.v – HTTP/Web constants and simulated web data
# Use these to conditionally compile networking features.

# HTTP methods
val HTTP_GET    = 1
val HTTP_POST   = 2
val HTTP_PUT    = 3
val HTTP_DELETE = 4

# Common HTTP status codes
val HTTP_OK                    = 200
val HTTP_CREATED               = 201
val HTTP_BAD_REQUEST           = 400
val HTTP_UNAUTHORIZED          = 401
val HTTP_FORBIDDEN             = 403
val HTTP_NOT_FOUND             = 404
val HTTP_INTERNAL_SERVER_ERROR = 500

# Default server settings
val HTTP_DEFAULT_PORT = 80
val HTTPS_DEFAULT_PORT = 443

# Example remote endpoints (hardcoded for compile‑time decisions)
val API_SERVER = "api.example.com"
val API_PATH   = "/v1/data"
val API_KEY    = "abc123"   # (not secure, just for demonstration)

# Timeout values (in seconds)
val HTTP_CONNECT_TIMEOUT = 10
val HTTP_READ_TIMEOUT    = 30

# Simulated response codes (can be used in if statements)
# For instance, you could have:
# if USE_WEB_FEATURES
#     val server_response = HTTP_OK
# else
#     val server_response = HTTP_NOT_FOUND
# end
"""
}

def main():
    # Create modules directory if it doesn't exist
    os.makedirs(MODULES_DIR, exist_ok=True)

    for filename, content in files.items():
        filepath = os.path.join(MODULES_DIR, filename)
        # Open with UTF-8 encoding to handle any Unicode characters
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(content)
        print(f"Created {filepath}")

    print("\nAll module files generated successfully in the 'modules/' folder.")

if __name__ == "__main__":
    main()