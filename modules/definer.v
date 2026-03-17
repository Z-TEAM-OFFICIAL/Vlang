# definer.v – Common game constants and utility definitions

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
