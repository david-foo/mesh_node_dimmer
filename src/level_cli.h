#ifndef _ONOFF_CLI_H
#define _ONOFF_CLI_H

// operations supported by this model. opcode, min msglen, message handler
#define BT_MESH_MODEL_OP_GEN_LEVEL_GET BT_MESH_MODEL_OP_2(0x82, 0x05)
#define BT_MESH_MODEL_OP_GEN_LEVEL_SET BT_MESH_MODEL_OP_2(0x82, 0x06)
#define BT_MESH_MODEL_OP_GEN_LEVEL_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x07)
#define BT_MESH_MODEL_OP_GEN_LEVEL_STATUS BT_MESH_MODEL_OP_2(0x82, 0x08)
#define BT_MESH_MODEL_OP_GEN_DELTA_SET BT_MESH_MODEL_OP_2(0x82, 0x09)
#define BT_MESH_MODEL_OP_GEN_DELTA_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x0A)
#define BT_MESH_MODEL_OP_GEN_MOVE_SET BT_MESH_MODEL_OP_2(0x82, 0x0B)
#define BT_MESH_MODEL_OP_GEN_MOVE_SET_UNACK BT_MESH_MODEL_OP_2(0x82, 0x0C)


struct level_state {
	s16_t current;
	void (*on_new_state)(u8_t state);
};

extern struct level_state level_cli_state;

extern const struct bt_mesh_model_op gen_level_cli_op[];

extern void gen_level_get(struct bt_mesh_model *model);
extern void gen_level_set(struct bt_mesh_model *model, s16_t level, u32_t delay, s32_t trans_time);
extern void gen_level_set_unack(struct bt_mesh_model *model, s16_t level, u32_t delay, s32_t trans_time);
extern void gen_delta_set(struct bt_mesh_model *model,  s32_t delta_level);
extern void gen_delta_set_unack(struct bt_mesh_model *model,  s32_t delta_level);
extern void gen_move_set(struct bt_mesh_model *model,  s32_t delta_level);
extern void gen_move_set_unack(struct bt_mesh_model *model,  s32_t delta_level);

extern void level_cli_init();



#endif