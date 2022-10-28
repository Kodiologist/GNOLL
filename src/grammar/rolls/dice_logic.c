#include "rolls/dice_logic.h"

#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "external/pcg_basic.h"
#include "rolls/dice_enums.h"
#include "shared_header.h"
#include "util/safe_functions.h"
#include "yacc_header.h"

#if USE_SECURE_RANDOM == 1
#include <bsd/stdlib.h>
#endif

#define EXPLOSION_LIMIT 50

extern pcg32_random_t rng;

int random_fn_run_count = 0;
int global_mock_value = 0;
int secondary_mock_value = 0;
MOCK_METHOD global_mock_style = NO_MOCK;
extern int gnoll_errno;

void reset_mocking() {
  /**
   * @brief Resets various globals for test mocking
   */
  random_fn_run_count = 0;
  global_mock_value = 0;
  global_mock_style = NO_MOCK;
}
void init_mocking(MOCK_METHOD mock_style, int starting_value) {
  /**
   * @brief Initializes test mocking with given settings
   * @param mock_style How to apply mocking
   * @param starting_value Where mocking is applied, sets the value for the
   * first roll on the system
   */
  random_fn_run_count = 0;
  global_mock_value = starting_value;
  global_mock_style = mock_style;
}

void mocking_tick() {
  /**
   * @brief Every time a dice is rolled, this function is called so that the
   * mocking logic can update
   */
  switch (global_mock_style) {
    case RETURN_INCREMENTING: {
      global_mock_value = global_mock_value + 1;
      break;
    }
    case RETURN_DECREMENTING: {
      global_mock_value = global_mock_value - 1;
      break;
    }
    case RETURN_CONSTANT_TWICE_ELSE_CONSTANT_ONE: {
      if (random_fn_run_count == 1) {
        secondary_mock_value = global_mock_value;
      }
      if (random_fn_run_count < 2) {
        global_mock_value = secondary_mock_value;
      } else {
        global_mock_value = 1;
      }
      break;
    }
    default:
      break;
  }
}

int random_fn(int small, int big) {
  /**
   * @brief Get a random number between 'small' and 'big'
   * @param small lower value
   * @param big higher value
   */
  if (gnoll_errno) {
    return 0;
  }

  // printf("Between %i and %i\n", small, big);
  random_fn_run_count++;

  if (small == big) {
    return big;
  }
  if (small > big) {
    // e.g. roll a minus sided die.
    // Roll between 1 and 0 -> 0
    // Roll a d-2 (1 and -2)
    return small;
  };

  int value = 0;
  if (global_mock_style == NO_MOCK) {
#if USE_SECURE_RANDOM == 1
    value = (int)arc4random_uniform(INT_MAX);
#else
    value = (int)pcg32_boundedrand_r(&rng, INT_MAX);
#endif
    value = value % (big + 1 - small) + small;
  } else {
    value = global_mock_value;
    mocking_tick();
  }
  // printf("Dice Roll Value: %i\n", value);
  return value;
}

// TODO: have min and max range rather than sides and start_offset

int* perform_roll(unsigned int number_of_dice, unsigned int die_sides,
                  EXPLOSION_TYPE explode, int start_value) {
  /**
   * @brief Controls logic of dice rolling above basic dX
   * @param number_of_dice - How many dice to roll
   * @param die_sides - How many sides a dice has
   * @param explode - What (if any) type of explosion logic to apply
   * @param start_value - Offset the roll results by this amojunt
   * @return Numeric Summation of all dice rolled in this fn
   */
  if (gnoll_errno) {
    return NULL;
  }

  int explosion_condition_score = 0;
  int explosion_count = 0;

  int* all_dice_roll = safe_calloc(number_of_dice, sizeof(int));
  if (gnoll_errno) {
    return 0;
  }

  int single_die_roll = 0;
  int exploded_result = 0;

  for (unsigned int i = 0; i < number_of_dice; i++) {
    all_dice_roll[i] = 0;
  }

  do {
    int end_value = (int)start_value + (int)die_sides - 1;
    for (unsigned int i = 0; i < number_of_dice; i++) {
      if (die_sides == 0) {
        break;
      }
      // printf("Roll between %d and %d\n", start_value, end_value);
      single_die_roll = random_fn(start_value, end_value);
      all_dice_roll[i] += single_die_roll;
      exploded_result += single_die_roll;
    }

    explosion_condition_score += (int)number_of_dice * (int)die_sides;
    if (explode != NO_EXPLOSION) {
      if (explode == ONLY_ONCE_EXPLOSION && explosion_count > 0) {
        break;
      }
      if (explode == PENETRATING_EXPLOSION) {
        die_sides--;
        if (die_sides == 0) {
          break;
        }
      }
      explosion_count++;
    } else {
      break;
    }
  } while (explode && (exploded_result == explosion_condition_score) &&
           explosion_count < EXPLOSION_LIMIT);

  return all_dice_roll;
}

int* do_roll(roll_params rp) {
  /**
   * @brief Unfurls the roll_params struct and calls dice rolling logic
   */
  return perform_roll(rp.number_of_dice, rp.die_sides, rp.explode,
                      rp.start_value);
}