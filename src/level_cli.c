#include <sys/printk.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include "board.h"
#include "main.h"
#include "level_cli.h"


static void gen_level_status(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf);

const struct bt_mesh_model_op gen_level_cli_op[] = {
	{ BT_MESH_MODEL_OP_GEN_LEVEL_STATUS, 1, gen_level_status },
	BT_MESH_MODEL_OP_END,
};

static void mb_on_new_state(u8_t state)
{
	printk("mb_on_new_state");
}

struct level_state level_cli_state = {
	.current = 0,
	.on_new_state = mb_on_new_state
};

static u8_t tid = 0;


void gen_level_get(struct bt_mesh_model *model)
{
	struct net_buf_simple *msg = model->pub->msg;
	int err;

	bt_mesh_model_msg_init(msg, BT_MESH_MODEL_OP_GEN_LEVEL_GET);

	err = bt_mesh_model_publish(model);
	if (err) {
		printk("bt_mesh_model_publish err %d\n", err);
	}
}

static u8_t time2tt(u32_t timems)
{
	u8_t steps = 0;
	u8_t resolution = 0;
	if (timems > 620000)
	{
		// > 620 seconds -> resolution=0b11 [10 minutes]
		resolution = 0x03;
		steps = timems / 600000;
	}
	else if (timems > 62000)
	{
		// > 62 seconds -> resolution=0b10 [10 seconds]
		resolution = 0x02;
		steps = timems / 10000;
	}
	else if (timems > 6200)
	{
		// > 6.2 seconds -> resolution=0b01 [1 seconds]
		resolution = 0x01;
		steps = timems / 1000;
	}
	else
	{
		// <= 6.2 seconds -> resolution=0b00 [100 ms]
		resolution = 0x00;
		steps = timems / 100;
	}
	printk("calculated steps=%d,resolution=%d\n", steps, resolution);
	return ((resolution << 6) | steps);
}

static void pub_level_set(struct bt_mesh_model *model, s16_t level, u32_t opcode, u32_t delay, s32_t trans_time)
{
	struct net_buf_simple *msg = model->pub->msg;
	int err;
	u8_t dy = delay / 5;
	u8_t tt = time2tt(trans_time);

	bt_mesh_model_msg_init(msg, opcode);
	net_buf_simple_add_le16(msg, level);
	net_buf_simple_add_u8(msg, tid++);
	net_buf_simple_add_u8(msg, tt);
	net_buf_simple_add_u8(msg, dy);
	err = bt_mesh_model_publish(model);
	if (err) {
		printk("bt_mesh_model_publish err %d\n", err);
		return;
	}
}

void gen_level_set(struct bt_mesh_model *model, s16_t level, u32_t delay, s32_t trans_time)
{
	pub_level_set(model, level, BT_MESH_MODEL_OP_GEN_LEVEL_SET, delay, trans_time);
}

void gen_level_set_unack(struct bt_mesh_model *model, s16_t level, u32_t delay, s32_t trans_time)
{
	pub_level_set(model, level, BT_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK, delay, trans_time);
}

static void pub_delta_set(struct bt_mesh_model *model, s32_t delta_level, u32_t opcode)
{
	struct net_buf_simple *msg = model->pub->msg;
	int err;

	bt_mesh_model_msg_init(msg, opcode);
	net_buf_simple_add_le32(msg, delta_level);
	net_buf_simple_add_u8(msg, tid++);
	err = bt_mesh_model_publish(model);
	if (err) {
		printk("bt_mesh_model_publish err %d\n", err);
		return;
	}
}

void gen_delta_set(struct bt_mesh_model *model,  s32_t delta_level)
{
	pub_delta_set(model, delta_level, BT_MESH_MODEL_OP_GEN_DELTA_SET);
}

void gen_delat_set_unack(struct bt_mesh_model *model,  s32_t delta_level)
{
	pub_delta_set(model, delta_level, BT_MESH_MODEL_OP_GEN_DELTA_SET_UNACK);
}

void gen_move_set(struct bt_mesh_model *model,  s32_t delta_level)
{

}

void gen_move_set_unack(struct bt_mesh_model *model,  s32_t delta_level)
{

}



static void gen_level_status(struct bt_mesh_model *model,
			  struct bt_mesh_msg_ctx *ctx,
			  struct net_buf_simple *buf)
{
	u16_t present_state = net_buf_simple_pull_le16(buf);
	u16_t target_state = net_buf_simple_pull_le16(buf);
	u8_t remaining_time = net_buf_simple_pull_u8(buf);

	printk("Level client model of element(%04x) recived new states"
		"from: %04x with present_state: %d target_state:%d remaining_time:%d\n",
		bt_mesh_model_elem(model)->addr, ctx->addr, present_state,
		target_state,remaining_time);
}

void level_cli_init()
{

}
