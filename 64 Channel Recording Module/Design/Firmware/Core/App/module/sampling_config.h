/**
 * @file sampling_config.h
 * @brief Compile-time sampling mode for Intan RHD2164 + STM32U5A5
 *
 * Select exactly ONE mode below, then rebuild/flash.
 * SMART "Number of Channels" must match ACTIVE_TOTAL_CHANNELS.
 */

#ifndef SAMPLING_CONFIG_H
#define SAMPLING_CONFIG_H

#include <stdint.h>

//=============================================================================
// MODE SELECT — uncomment exactly one
//=============================================================================

/** 64 channels @ 2 kHz (UART ~2.6 Mbps @ 3 Mbaud, 132-byte framed packets) */
// #define SAMPLING_MODE_ALL_64_2KHZ    1

/** 1 channel @ ~30 kHz (UART ~1.8 Mbps @ 3 Mbaud, 6-byte framed packets) */
#define SAMPLING_MODE_SINGLE_30KHZ  1

/** Used only by SAMPLING_MODE_SINGLE_30KHZ: Intan channel index 0–63
 *  (0–31 = bank A / MISO A, 32–63 = bank B / MISO B) */
#define SINGLE_CHANNEL_INDEX         18

//=============================================================================
// Derived settings (do not edit unless you know why)
//=============================================================================

#if defined(SAMPLING_MODE_ALL_64_2KHZ) && defined(SAMPLING_MODE_SINGLE_30KHZ)
#error "Define only one of SAMPLING_MODE_ALL_64_2KHZ or SAMPLING_MODE_SINGLE_30KHZ"
#endif

#if !defined(SAMPLING_MODE_ALL_64_2KHZ) && !defined(SAMPLING_MODE_SINGLE_30KHZ)
#error "Define SAMPLING_MODE_ALL_64_2KHZ or SAMPLING_MODE_SINGLE_30KHZ in sampling_config.h"
#endif

/* Timer: 160 MHz / ((159+1) * (PERIOD+1)) = 1e6 / (PERIOD+1) Hz */
#define TIMER_PRESCALER              159

#if defined(SAMPLING_MODE_ALL_64_2KHZ)

#define CFG_SAMPLE_RATE_HZ           2000u
#define TIMER_PERIOD                 499   /* 1e6 / 500 = 2000 Hz */
#define ACTIVE_CHANNELS_PER_STREAM   32
#define ACTIVE_TOTAL_CHANNELS        64
#define SPI_WORDS_PER_SCAN           32
#define CFG_SINGLE_CHANNEL_MODE      0
#define CFG_BANK_B_ENABLED           1
#define CFG_USE_MISO_A               1
#define CFG_USE_MISO_B               1
#define CFG_CHIP_CHANNEL             0
#define START_CHANNEL_A              0
#define END_CHANNEL_A                31
#define START_CHANNEL_B              0
#define END_CHANNEL_B                31
#define USE_MISO_B                   1

#elif defined(SAMPLING_MODE_SINGLE_30KHZ)

#if (SINGLE_CHANNEL_INDEX < 0) || (SINGLE_CHANNEL_INDEX > 63)
#error "SINGLE_CHANNEL_INDEX must be 0..63"
#endif

#define CFG_SAMPLE_RATE_HZ           30303u
#define TIMER_PERIOD                 32    /* 1e6 / 33 ≈ 30303 Hz */
#define ACTIVE_CHANNELS_PER_STREAM   1
#define ACTIVE_TOTAL_CHANNELS        1
/** 3× CONVERT(ch) per tick; MISO word[2] is the settled ADC result */
#define SPI_WORDS_PER_SCAN           3
#define SPI_RESULT_WORD_INDEX        2
#define CFG_SINGLE_CHANNEL_MODE      1

#if (SINGLE_CHANNEL_INDEX < 32)
#define CFG_CHIP_CHANNEL             (SINGLE_CHANNEL_INDEX)
#define CFG_BANK_B_ENABLED           0
#define CFG_USE_MISO_A               1
#define CFG_USE_MISO_B               0
#define START_CHANNEL_A              (SINGLE_CHANNEL_INDEX)
#define END_CHANNEL_A                (SINGLE_CHANNEL_INDEX)
#define START_CHANNEL_B              0
#define END_CHANNEL_B                0
#define USE_MISO_B                   0
#else
#define CFG_CHIP_CHANNEL             (SINGLE_CHANNEL_INDEX - 32)
#define CFG_BANK_B_ENABLED           1
#define CFG_USE_MISO_A               0
#define CFG_USE_MISO_B               1
#define START_CHANNEL_A              0
#define END_CHANNEL_A                0
#define START_CHANNEL_B              (SINGLE_CHANNEL_INDEX - 32)
#define END_CHANNEL_B                (SINGLE_CHANNEL_INDEX - 32)
#define USE_MISO_B                   1
#endif

#endif /* mode */

#if CFG_SINGLE_CHANNEL_MODE
/** Pipeline delay is satisfied inside the 3-word burst; no scan skip needed */
#define INTAN_PIPELINE_SKIP_SCANS    0
#else
/** Reserved / documentary for multi-channel start lag (not used in 64ch TX path) */
#define INTAN_PIPELINE_SKIP_SCANS    2
#endif

#endif /* SAMPLING_CONFIG_H */
