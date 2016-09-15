#ifndef __ONEWAY_ANCHOR_H
#define __ONEWAY_ANCHOR_H

#include "deca_device_api.h"
#include "deca_regs.h"

#include "dw1000.h"
#include "prng.h"

// Set at some arbitrary length for what the longest packet we will receive
// is.
#define ONEWAY_ANCHOR_MAX_RX_PKT_LEN 64

typedef enum {
	ASTATE_IDLE,
	ASTATE_RANGING,
	ASTATE_RESPONDING
} oneway_anchor_state_e;

typedef enum {
	RSTATE_IDLE,
	RSTATE_BROADCASTS,
	RSTATE_TRANSITION_TO_ANC_FINAL,
	RSTATE_LISTENING,
	RSTATE_CALCULATE_RANGE
} ranging_state_e;

// Configuration data for the ANCHOR provided by the TAG
typedef struct {
	uint8_t  reply_after_subsequence;
	uint32_t anchor_reply_window_in_us;
	uint16_t anchor_reply_slot_time_in_us;
	uint16_t anchor_reply_num_slots;
} oneway_anchor_tag_config_t;

typedef struct {
	// Our timer object that we use for timing packet transmissions
	stm_timer_t* anchor_timer;
	
	// State for the PRNG
	ranctx prng_state;
	
	/******************************************************************************/
	// Keep track of state for the given ranging event this anchor is handling.
	/******************************************************************************/
	// What the anchor is currently doing
	oneway_anchor_state_e state;
	ranging_state_e ranging_state;
	// Which spot in the ranging broadcast sequence we are currently at
	uint8_t ranging_broadcast_ss_num;
	// What config parameters the tag sent us
	oneway_anchor_tag_config_t ranging_operation_config;
	// Which spot in the listening window sequence we are in.
	// The listening window refers to the time after the ranging broadcasts
	// when the tag listens for anchor responses on each channel
	uint8_t ranging_listening_window_num;
	
	// Keep track of, in each ranging session with a tag, how many packets we
	// receive on each antenna. This lets us pick the best antenna to use
	// when responding to a tag.
	uint8_t anchor_antenna_recv_num[NUM_ANTENNAS];
	
	// Array of when we sent each of the broadcast ranging packets
	uint64_t ranging_broadcast_ss_send_times[NUM_RANGING_BROADCASTS];
	
	// How many anchor responses we have gotten
	uint8_t anchor_response_count;
	
	// Array of when we received ANC_FINAL packets and from whom
	anchor_responses_t anchor_responses[MAX_NUM_ANCHOR_RESPONSES];

	// These are the ranges we have calculated to a series of anchors.
	// They use the same index as the _anchor_responses array.
	// Invalid ranges are marked with INT32_MAX.
	int32_t ranges_millimeters[MAX_NUM_ANCHOR_RESPONSES];
	
	// Prepopulated struct of the outgoing broadcast poll packet.
	struct pp_tag_poll pp_tag_poll_pkt;
	
	// Packet that the anchor unicasts to the tag
	struct pp_anc_final pp_anc_final_pkt;

	bool final_ack_received;
} oneway_anchor_scratchspace_struct;

oneway_anchor_scratchspace_struct *oa_scratch;

void oneway_anchor_init (void *app_scratchspace);
dw1000_err_e oneway_anchor_start ();
dw1000_err_e oneway_anchor_start_ranging_event ();
void oneway_anchor_stop ();

#endif
