// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

enum layers {
    _QWERTY = 0,
    _SYM,
    _MEDIA,
};

static uint16_t animation_timer;

bool is_oled_active;

void housekeeping_task_user() {
    is_oled_active = last_input_activity_elapsed() < 60000;
}

void keyboard_post_init_user() {
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    oled_clear();

    animation_timer = timer_read();
    srand(timer_read());
}

bool rgb_matrix_indicators_user(void) {
    switch (get_highest_layer(layer_state)) {
        case _SYM:
            rgb_matrix_set_color_all(RGB_AZURE);
            break;

        case _MEDIA:
            rgb_matrix_set_color_all(RGB_CHARTREUSE);
            break;

        case _QWERTY:
        default:
            rgb_matrix_set_color_all(RGB_CORAL);
    }

    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(11, RGB_RED);
    }
    return false;
}

// Aliases for readability
#define SYM MO(_SYM)
#define MEDIA MO(_MEDIA)
#define FKEYS MO(_FUNCTION)
#define ADJUST MO(_ADJUST)

#define CTL_ESC MT(MOD_LCTL, KC_ESC)
#define CTL_QUOT MT(MOD_RCTL, KC_QUOTE)
#define CTL_MINS MT(MOD_RCTL, KC_MINUS)
#define ALT_ENT MT(MOD_LALT, KC_ENT)

#define SFT_ESC LSFT_T(KC_ESCAPE)
#define L1_ENT LT(1, KC_ENT)

// Left and right braces
#define LPBRCK KC_NONUS_BACKSLASH
#define RPBRCK RSFT(KC_NONUS_BACKSLASH)
#define LBRC RALT(KC_LBRC)
#define RBRC RALT(KC_RBRC)
#define LCURL RALT(KC_QUOTE)
#define RCURL RALT(KC_BSLS)
#define LPAREN LSFT(KC_8)
#define RPAREN LSFT(KC_9)

// Note: LAlt/Enter (ALT_ENT) is not the same thing as the keyboard shortcut Alt+Enter.
// The notation `mod/tap` denotes a key that activates the modifier `mod` when held down, and
// produces the key `tap` when tapped (i.e. pressed and released).

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * Base Layer: QWERTY
 *
 * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
 * |  Esc   |   1  |   2  |   3  |   4  |   5  |      |LShift|  |RShift|      |   6  |   7  |   8  |   9  |   0  |   '    |
 * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
 * |  Tab   |   Q  |   W  |   E  |   R  |   T  |      |LCtrl |  | RCtrl|      |   Y  |   U  |   I  |   O  |   P  |  Bksp  |
 * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
 * |LShf/Esc|   A  |   S  |   D  |   F  |   G  |      | LAlt |  | RAlt |      |   H  |   J  |   K  |   L  | ;  : | RShift |
 * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
 * |  LCtrl |   Z  |   X  |   C  |   V  |   B  | Del  |MO(2) |  |C-lock|  ] } |   N  |   M  | ,    | .    | -    |  RCtl  |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        | SYM  | LGUI | LAlt | Bksp | TD(0)|  |MO(2) | Space| AltGr| RGUI | Menu |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 *
 * ,----------------------------.      ,------.                 ,----------------------------.      ,------.
 * | Prev | Next | Pause | Stop |      | Mute |                 | Prev | Next | Pause | Stop |      | Mute |
 * `----------------------------'      `------'                 `----------------------------'      '------'
 */
    [_QWERTY] = LAYOUT_myr(
      KC_ESC  , KC_1 ,  KC_2   ,  KC_3  ,   KC_4 ,   KC_5 ,         KC_LSFT,     KC_RSFT,           KC_6 ,  KC_7 ,  KC_8 ,   KC_9 ,  KC_0 , KC_MINUS,
      KC_TAB  , KC_Q ,  KC_W   ,  KC_E  ,   KC_R ,   KC_T ,         KC_LCTL,     KC_RCTL,           KC_Y ,  KC_U ,  KC_I ,   KC_O ,  KC_P , KC_BSPC,
      SFT_ESC , KC_A ,  KC_S   ,  KC_D  ,   KC_F ,   KC_G ,         KC_LALT,     KC_RALT,           KC_H ,  KC_J ,  KC_K ,   KC_L ,KC_SCLN, KC_RSFT,
      KC_LCTL , KC_Z ,  KC_X   ,  KC_C  ,   KC_V ,   KC_B , KC_DEL, MEDIA  ,     KC_CAPS , KC_RBRC, KC_N ,  KC_M ,KC_COMM, KC_DOT ,KC_SLSH, KC_RCTL,
                                           SYM    , KC_LGUI, KC_LALT, KC_BSPC, L1_ENT,     MEDIA   , KC_SPC ,KC_RALT, KC_RGUI, KC_APP,

      _______, _______, _______, _______,    _______,                            KC_VOLU, KC_VOLD, _______, _______,    KC_MPLY
    ),

/*
 * Layer template - LAYOUT_myr
 *
 * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
 * |        |  <   |  [   |  {   |  (   |      |      |      |  |      |      |      |   )  |   }  |   ]  |   >  |        |
 * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |   ←  |   ↓  |   ↑  |   →  |      |        |
 * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 *
 * ,-----------------------------.      ,------.                ,---------------------------.      ,------.
 * |        |      |      |      |      |      |                |      |      |      |      |      |      |
 * `-----------------------------'      `------'                `---------------------------'      '------'
 */
    [_SYM] = LAYOUT_myr(
      _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, LPBRCK , LBRC   , LCURL  , LPAREN , _______,          _______, _______,          _______, RPAREN , RCURL  , RBRC   , RPBRCK , _______,
      _______, _______, _______, _______, _______, _______,          _______, _______,          KC_LEFT, KC_DOWN, KC_UP  , KC_RGHT, _______, _______,
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_LBRC, KC_RBRC, KC_QUOT, KC_EQL , _______,
                                 _______, _______, _______       , _______   , KC_NO  , _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   KC_TRNS, KC_TRNS, _______, _______,          KC_TRNS
    ),

/*
 * Layer template - LAYOUT_myr
 *
 * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
 * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
 * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        |      |      |      |      |      |  |      |      |      |      |      |
 *                        `----------------------------------'  `----------------------------------'
 *
 * ,-----------------------------.      ,------.                ,---------------------------.      ,------.
 * |        |      |      |      |      |      |                |      |      |      |      |      |      |
 * `-----------------------------'      `------'                `---------------------------'      '------'
 */
    [_MEDIA] = LAYOUT_myr(
      _______, _______, _______      , _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, _______, _______      , _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, KC_PSCR, LGUI(KC_PSCR), _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
      _______, KC_MPRV, KC_MNXT      , _______, _______, KC_MPLY, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                                 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,

      _______, _______, _______, _______,          _______,                   KC_TRNS, KC_TRNS, _______, _______,          KC_TRNS
    ),

// /*
//  * Layer template - LAYOUT
//  *
//  * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        `----------------------------------'  `----------------------------------'
//  */
//     [_LAYERINDEX] = LAYOUT(
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
//     ),

// /*
//  * Layer template - LAYOUT_myr
//  *
//  * ,-------------------------------------------.      ,------.  ,------.      ,-------------------------------------------.
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------|      |------|  |------|      |------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * |--------+------+------+------+------+------+------+------|  |------|------+------+------+------+------+------+--------|
//  * |        |      |      |      |      |      |      |      |  |      |      |      |      |      |      |      |        |
//  * `----------------------+------+------+------+------+------|  |------+------+------+------+------+----------------------'
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        |      |      |      |      |      |  |      |      |      |      |      |
//  *                        `----------------------------------'  `----------------------------------'
//  *
//  * ,-----------------------------.      ,------.                ,---------------------------.      ,------.
//  * |        |      |      |      |      |      |                |      |      |      |      |      |      |
//  * `-----------------------------'      `------'                `---------------------------'      '------'
//  */
//     [_LAYERINDEX] = LAYOUT_myr(
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//
//       _______, _______, _______, _______,          _______,                   _______, _______, _______, _______,          _______
//     ),
};

/* The default OLED and rotary encoder code can be found at the bottom of qmk_firmware/keyboards/splitkb/elora/rev1/rev1.c
 * These default settings can be overriden by your own settings in your keymap.c
 * DO NOT edit the rev1.c file; instead override the weakly defined default functions by your own.
 */

// clang-format on

#define START_POSITION {3, 8}

#define TILE_SIZE_X 8
#define TILE_SIZE_Y 1
#define SPRITE_SIZE ((TILE_SIZE_X * 2) * (TILE_SIZE_Y * 2))

#ifdef OLED_ENABLE
typedef struct position_s {
    uint16_t x;
    uint16_t y;
} position_t;

typedef enum direction_e {
    LEFT = 0,
    RIGHT,
} direction_t;

typedef enum sprite_type_e {
    IDLE = 0,
} sprite_type_t;

typedef const char PROGMEM sprite_t[SPRITE_SIZE];

typedef enum animation_type_e {
    NONE = 0,
    TURN,
    WALK,
    ASCEND,
    DESCEND,
    ANIMATION_MAX,
} animation_type_t;

typedef struct animation_s {
    animation_type_t type;
    uint16_t         step;
} animation_t;

#    define ANIMATION_INIT {.type = NONE, .step = 0}

void animation_reset(animation_t* a) {
    a->type = NONE;
    a->step = 0;
}

typedef struct digimon_s {
    position_t  position;
    direction_t direction;
    animation_t animation;
    sprite_t    idle;
} digimon_t;

uint16_t digimon_position(const digimon_t* d) {
    return d->position.x * 8 + d->position.y * 64;
}

void digimon_direction_toggle(digimon_t* d) {
    if (d->direction == LEFT) {
        d->direction = RIGHT;
    } else {
        d->direction = LEFT;
    }
}

const char* digimon_sprite(const digimon_t* d, sprite_type_t type) {
    switch (type) {
        case IDLE:
            return d->idle;
    }
    return d->idle;
}

/**
 * Clear the frames where the digimon is currently standing
 */
void digimon_clear(digimon_t* d) {
    for (int i = 0; i < SPRITE_SIZE; i++) {
        uint16_t offset = i < 16 ? 0 : 48;
        oled_write_raw_byte(0x00, digimon_position(d) + i + offset);
    }
}

void digimon_render(digimon_t* d) {
    const char* sprite = digimon_sprite(d, IDLE);

    int      i      = 0;
    int      j      = d->direction == LEFT ? 0 : 15;
    uint16_t offset = 0;
    while (i < SPRITE_SIZE) {
        if (i == 16) {
            offset = 48; // Move to the next line.
            if (d->direction == RIGHT) {
                j = 31;
            }
        }

        oled_write_raw_byte(sprite[j], digimon_position(d) + i + offset);

        i++;
        if (d->direction == LEFT) {
            j++;
        } else {
            j--;
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
    if (d->direction == LEFT && d->position.x > 0) {
        d->position.x--;
    } else if (d->direction == RIGHT && d->position.x < 6) {
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

bool oled_task_user(void) {
    static digimon_t PROGMEM agumon = {
        .position  = START_POSITION,
        .direction = LEFT,
        .animation = ANIMATION_INIT,
        // clang-format off
        .idle = {
        0x70, 0x88, 0x88, 0x88, 0x8c, 0x4e, 0x46, 0x02,
        0x2a, 0x3a, 0x32, 0x04, 0xc8, 0x70, 0x00, 0x00,
        0x00, 0xcd, 0xaa, 0xea, 0x9e, 0x92, 0xe2, 0x2d,
        0xeb, 0xaa, 0xd8, 0x83, 0xfc, 0xa8, 0xd8, 0x00
        } // clang-format on
    };

    // Turn off oled if more than 60 seconds without activity go by
    if (is_oled_active) {
        oled_on();
    } else {
        oled_off();
        return false;
    }

    if (timer_elapsed(animation_timer) > 500) {
        animation_timer = timer_read();
        digimon_clear(&agumon);

        if (agumon.animation.type == NONE) {
            // Pick a new animation, but exclude NONE
            agumon.animation.type = (random() % (ANIMATION_MAX - 1)) + 1;
        }

        switch (agumon.animation.type) {
            case TURN:
                digimon_turn(&agumon);
                break;

            case WALK:
                digimon_walk(&agumon);
                break;

            case ASCEND:
                digimon_vertical_move(&agumon, true);
                break;

            case DESCEND:
                digimon_vertical_move(&agumon, false);
                break;

            case NONE:
            case ANIMATION_MAX:
                // unreachable
                break;
        }

        digimon_render(&agumon);
    }

    return false;
}
#endif

/* DELETE THIS LINE TO UNCOMMENT (1/2)
#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
  // Your code goes here
}
#endif
DELETE THIS LINE TO UNCOMMENT (2/2) */
