#include "digimon.h"

#define START_POSITION {3, 8}

#define TILE_SIZE_X 8
#define TILE_SIZE_Y 1
#define TILE_SIZE_TOTAL (TILE_SIZE_X * TILE_SIZE_Y)
#define ATTACK_SIZE (TILE_SIZE_TOTAL)

uint16_t animation_timer;
uint32_t last_activity_elapsed;

typedef struct position_s {
    uint16_t x;
    uint16_t y;
} position_t;

void position_print(position_t pos) {
    oled_write("X: ", false);
    oled_write_char(pos.x + '0', false);
    oled_write_ln("", false);
    oled_write("Y: ", false);
    oled_write_char((pos.y / 10) + '0', false);
    oled_write_char((pos.y % 10) + '0', false);
    oled_write_ln("", false);
}

typedef enum direction_e {
    DIRECTION_LEFT = 0,
    DIRECTION_RIGHT,
} direction_t;

typedef char PROGMEM tile_t[TILE_SIZE_TOTAL];

void tile_render(const tile_t tile, position_t pos, bool invert) {
    int i = 0;
    int j = invert ? 7 : 0;
    while (i < TILE_SIZE_TOTAL) {
        oled_write_raw_byte(tile[j], (pos.x * 8 + pos.y * 64) + i);

        i++;
        if (invert) {
            j--;
        } else {
            j++;
        }
    }
}

void tile_clear(position_t pos) {
    for (int i = 0; i < TILE_SIZE_TOTAL; i++) {
        oled_write_raw_byte(0x00, (pos.x * 8 + pos.y * 64) + i);
    }
}

typedef enum sprite_type_e {
    SPRITE_IDLE = 0,
    SPRITE_ATTACK,
    SPRITE_SLEEP,
    SPRITE_HAPPY,
    SPRITE_MAX,
} sprite_type_t;

#define SPRITE_SIZE 4

typedef tile_t sprite_t[SPRITE_SIZE];

typedef enum animation_type_e {
    ANIMATION_NONE = 0,
    ANIMATION_TURN,
    ANIMATION_WALK,
    ANIMATION_ASCEND,
    ANIMATION_DESCEND,
    ANIMATION_MOVE_MARKER, // This is not an actual animation
    ANIMATION_ATTACK,
    ANIMATION_DANCE,
    ANIMATION_EAT,
    ANIMATION_MAX,
    // Sleep is excluded from the random set of animations
    ANIMATION_SLEEP,
} animation_type_t;

typedef struct animation_s {
    animation_type_t type;
    uint16_t         step;
} animation_t;

#define ANIMATION_INIT {.type = ANIMATION_NONE, .step = 0}

void animation_reset(animation_t* a) {
    a->type = ANIMATION_NONE;
    a->step = 0;
}

typedef struct digimon_s {
    position_t  position;
    direction_t direction;
    animation_t animation;
    sprite_t    sprites[SPRITE_MAX];

    const char PROGMEM attack[ATTACK_SIZE];
} digimon_t;

uint16_t digimon_position(const digimon_t* d) {
    return d->position.x * 8 + d->position.y * 64;
}

void digimon_direction_toggle(digimon_t* d) {
    if (d->direction == DIRECTION_LEFT) {
        d->direction = DIRECTION_RIGHT;
    } else {
        d->direction = DIRECTION_LEFT;
    }
}

const tile_t* digimon_sprite(const digimon_t* d) {
    switch (d->animation.type) {
        case ANIMATION_ATTACK:
            if (d->animation.step == 1) {
                return d->sprites[SPRITE_ATTACK];
            } else {
                break;
            }

        case ANIMATION_SLEEP:
            return d->sprites[SPRITE_SLEEP];

        case ANIMATION_DANCE:
            return d->sprites[SPRITE_HAPPY];

        case ANIMATION_EAT:
            if (d->animation.step % 2 != 0) {
                return d->sprites[SPRITE_HAPPY];
            }
            return d->sprites[SPRITE_IDLE];

        default:
            break;
    }
    return d->sprites[SPRITE_IDLE];
}

/**
 * Clear the frames where the digimon is currently standing
 */
void digimon_clear(digimon_t* d) {
    position_t pos = d->position;
    for (int i = 0; i < SPRITE_SIZE; i++) {
        switch (i) {
            case 0:
                break;
            case 1:
                pos.x++;
                break;
            case 2:
                pos.y++;
                break;
            case 3:
                pos.x--;
                break;
        }
        tile_clear(pos);
    }
}

void digimon_render(digimon_t* d) {
    const tile_t* sprite = digimon_sprite(d);
    position_t    pos    = d->position;

    bool looking_right = d->direction == DIRECTION_RIGHT;

    int j = looking_right ? 1 : 0;
    for (int i = 0; i < SPRITE_SIZE; i++) {
        switch (i) {
            case 0:
                break;
            case 1:
                pos.x++;
                break;
            case 2:
                pos.x--;
                pos.y++;
                if (looking_right) {
                    j = 3;
                }
                break;
            case 3:
                pos.x++;
                break;
        }

        tile_render(sprite[j], pos, looking_right);

        if (looking_right) {
            j--;
        } else {
            j++;
        }
    }
}

/**
 * Look the other direction
 */
void digimon_turn(digimon_t* d) {
    digimon_direction_toggle(d);
    animation_reset(&d->animation);
}

/**
 * If possible, take a step forward, otherwise turn.
 */
void digimon_walk(digimon_t* d) {
    if (d->direction == DIRECTION_LEFT && d->position.x > 0) {
        d->position.x--;
    } else if (d->direction == DIRECTION_RIGHT && d->position.x < 6) {
        d->position.x++;
    } else {
        digimon_turn(d);
    }

    animation_reset(&d->animation);
}

/**
 * Move vertically if possible, otherwise go in the oposite direction
 */
void digimon_vertical_move(digimon_t* d, bool up) {
    if (up && d->position.y > 0) {
        d->position.y--;
    } else if (!up && d->position.y < 14) {
        d->position.y++;
    } else {
        digimon_vertical_move(d, !up);
    }

    animation_reset(&d->animation);
}

void digimon_attack_clear(const digimon_t* d, uint16_t offset) {
    position_t pos = d->position;
    if (d->direction == DIRECTION_RIGHT) {
        pos.x += 2 + offset;
    } else {
        pos.x -= 1 + offset;
    }
    tile_clear(pos);
    pos.y++;
    tile_clear(pos);
}

void digimon_attack_render(const digimon_t* d, uint16_t offset) {
    position_t pos = d->position;
    if (d->direction == DIRECTION_RIGHT) {
        pos.x += 2 + offset;
    } else {
        pos.x -= 1 + offset;
    }
    tile_render(d->attack, pos, d->direction == DIRECTION_RIGHT);
    pos.y++;
    tile_render(d->attack, pos, d->direction == DIRECTION_RIGHT);
}

void digimon_attack(digimon_t* d) {
    switch (d->animation.step) {
        case 0:
            // If we don't have enough room to attack, turn first
            if ((d->direction == DIRECTION_LEFT && d->position.x <= 1) || (d->direction == DIRECTION_RIGHT && d->position.x >= 5)) {
                digimon_direction_toggle(d);
            }

            // Render the attack next to the digimon
            digimon_attack_render(d, 0);
            break;
        case 1:
            digimon_attack_clear(d, 0);
            digimon_attack_render(d, 1);
            break;
        case 2:
            // mark the animation as done
            animation_reset(&d->animation);
            digimon_attack_clear(d, 1);
            break;
    }
    d->animation.step++;
}

void digimon_sleep(digimon_t* d) {
    const static tile_t zeds = {
        0x80, 0x20, 0x60, 0x00, 0x24, 0x34, 0x2c, 0x24,
    };

    // Make room for the zeds
    if (d->position.y == 0) {
        d->position.y++;
    }

    position_t pos = d->position;
    pos.y--;

    tile_clear(pos);
    pos.x++;
    tile_clear(pos);

    if (d->animation.step == 0) {
        pos.x--;
        d->animation.step = 1;
    } else {
        d->animation.step = 0;
    }

    tile_render(zeds, pos, false);
}

void digimon_dance(digimon_t* d) {
    if (d->animation.step == 4) {
        animation_reset(&d->animation);
        return;
    }

    digimon_direction_toggle(d);
    d->animation.step++;
}

void digimon_eat(digimon_t* d) {
    const static tile_t meat = {
        0x03, 0x1d, 0x3e, 0x7e, 0x7a, 0x54, 0xb8, 0xc0,
    };

    switch (d->animation.step) {
        case 0: {
            position_t pos = d->position;
            if ((d->direction == DIRECTION_LEFT && d->position.x == 0) || (d->direction == DIRECTION_RIGHT && d->position.x >= 5)) {
                // If we don't have enough room to eat, turn first
                digimon_direction_toggle(d);
            }

            if (d->direction == DIRECTION_LEFT) {
                pos.x--;
            } else {
                pos.x += 2;
            }
            tile_render(meat, pos, false);
            break;
        }
        case 5: {
            position_t pos = d->position;
            if (d->direction == DIRECTION_LEFT) {
                pos.x--;
            } else {
                pos.x += 2;
            }
            tile_clear(pos);

            // Dance after eating
            d->animation.step = 0;
            d->animation.type = ANIMATION_DANCE;
            return;
        }
    }

    d->animation.step++;
}

void digimon_animate(digimon_t* d) {
    if (last_activity_elapsed > 30000) {
        if (d->animation.type == ANIMATION_NONE) {
            d->animation.type = ANIMATION_SLEEP;
        }
    } else if (d->animation.type == ANIMATION_SLEEP) {
        position_t pos = d->position;
        pos.y--;
        tile_clear(pos);
        pos.x++;
        tile_clear(pos);
        d->animation.type = ANIMATION_NONE;
    }

    if (d->animation.type == ANIMATION_NONE) {
        uint32_t value = random() % 100;
        if (value < 85) {
            // Do a move animation
            d->animation.type = (random() % (ANIMATION_MOVE_MARKER - 1)) + 1;
        } else {
            // do something else
            d->animation.type = (random() % (ANIMATION_MAX - ANIMATION_MOVE_MARKER - 1)) + ANIMATION_MOVE_MARKER + 1;
        }
        // Pick a new animation, but exclude NONE
    }

    switch (d->animation.type) {
        case ANIMATION_TURN:
            digimon_turn(d);
            break;

        case ANIMATION_WALK:
            digimon_walk(d);
            break;

        case ANIMATION_ASCEND:
            digimon_vertical_move(d, true);
            break;

        case ANIMATION_DESCEND:
            digimon_vertical_move(d, false);
            break;

        case ANIMATION_ATTACK:
            digimon_attack(d);
            break;

        case ANIMATION_DANCE:
            digimon_dance(d);
            break;

        case ANIMATION_EAT:
            digimon_eat(d);
            break;

        case ANIMATION_SLEEP:
            digimon_sleep(d);
            break;

        case ANIMATION_NONE:
        case ANIMATION_MAX:
        case ANIMATION_MOVE_MARKER:
            // unreachable
            break;
    }
}

// clang-format off
static digimon_t PROGMEM agumon = {
    .position  = START_POSITION,
    .direction = DIRECTION_LEFT,
    .animation = ANIMATION_INIT,
    .sprites = {
        [SPRITE_IDLE] = {
            {0x60, 0xd0, 0x50, 0x48, 0x0c, 0x02, 0x2a, 0x3a},
            {0x32, 0x02, 0x04, 0x18, 0xe0, 0x00, 0x00, 0x00},
            {0x00, 0xc0, 0xad, 0xeb, 0x9f, 0x91, 0xe1, 0x20},
            {0xec, 0xaa, 0xd8, 0x83, 0xcc, 0xb0, 0xc0, 0x00},
        },
        [SPRITE_ATTACK] = {
            {0x04, 0x8a, 0x4a, 0x4a, 0x72, 0x01, 0x19, 0x1d},
            {0x05, 0x01, 0x02, 0x0c, 0xf0, 0x00, 0x00, 0x00},
            {0x00, 0xc0, 0xad, 0xeb, 0x9f, 0x91, 0xe1, 0x20},
            {0xec, 0xaa, 0xd8, 0x83, 0xcc, 0xb0, 0xc0, 0x00},
        },
        [SPRITE_SLEEP] = {
            {0x00, 0xc0, 0xa0, 0xa0, 0x90, 0x18, 0x04, 0x44},
            {0x84, 0x84, 0x04, 0x08, 0x30, 0xc0, 0x00, 0x00},
            {0x7e, 0x81, 0x99, 0xb7, 0xad, 0xb5, 0xad, 0xb5},
            {0xad, 0xb5, 0xad, 0xb5, 0xaf, 0x99, 0x81, 0x7e},
        },
        [SPRITE_HAPPY] = {
            {0x04, 0x8a, 0x4a, 0x4a, 0x72, 0x01, 0x09, 0x05},
            {0x05, 0x09, 0x02, 0x0c, 0xf0, 0x00, 0x00, 0x00},
            {0x00, 0xc0, 0xad, 0xeb, 0x9f, 0x91, 0xe1, 0x20},
            {0xec, 0xaa, 0xd8, 0x83, 0xcc, 0xb0, 0xc0, 0x00},
        },
    },
    .attack = {
        0x78, 0xcc, 0x84, 0x82, 0x87, 0xc4, 0x72, 0x1f,
    },
};
// clang-format on

bool oled_task_user(void) {
    // Turn off oled if more than 60 seconds without activity go by
    if (last_activity_elapsed < 60000) {
        oled_on();
    } else {
        oled_off();
        return false;
    }

    if (timer_elapsed(animation_timer) > 500) {
        animation_timer = timer_read();
        digimon_clear(&agumon);
        digimon_animate(&agumon);
        digimon_render(&agumon);
    }

    return false;
}
