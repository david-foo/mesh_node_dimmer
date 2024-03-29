/*
 * embest
 *
 *
 */

#include <zephyr.h>
#include <sys/printk.h>

#include <settings/settings.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/mesh.h>

#include <minode.h>

#include "board.h"
#include "level_cli.h"
#include "main.h"


u16_t primary_addr = BT_MESH_ADDR_UNASSIGNED;

static struct bt_mesh_cfg_srv cfg_srv = {
	.relay = BT_MESH_RELAY_ENABLED,
	.beacon = BT_MESH_BEACON_ENABLED,
#if defined(CONFIG_BT_MESH_FRIEND)
	.frnd = BT_MESH_FRIEND_ENABLED,
#else
	.frnd = BT_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BT_MESH_GATT_PROXY)
	.gatt_proxy = BT_MESH_GATT_PROXY_ENABLED,
#else
	.gatt_proxy = BT_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
	.default_ttl = 7,

	/* 3 transmissions with 20ms interval */
	.net_transmit = BT_MESH_TRANSMIT(2, 20),
	.relay_retransmit = BT_MESH_TRANSMIT(2, 20),
};

static struct bt_mesh_health_srv health_srv = {
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);
BT_MESH_MODEL_PUB_DEFINE(gen_level_pub_cli, NULL,  2 + 7 + 4);

struct bt_mesh_model root_models[] = {
	BT_MESH_MODEL_CFG_SRV(&cfg_srv),
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
	BT_MESH_MODEL(BT_MESH_MODEL_ID_GEN_LEVEL_CLI, gen_level_cli_op,
		&gen_level_pub_cli, &level_cli_state),
};

struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(0, root_models, BT_MESH_MODEL_NONE),
};

const struct bt_mesh_comp comp = {
	.cid = BT_COMP_ID_LF,
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

static int output_number(bt_mesh_output_action_t action, u32_t number)
{
	printk("OOB Number: %u\n", number);

	board_output_number(action, number);

	return 0;
}

static void prov_complete(u16_t net_idx, u16_t addr)
{
	primary_addr = addr;
	board_prov_complete();
	bt_mesh_proxy_identity_enable();
}

static void prov_reset(void)
{
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);
}

static const uint8_t dev_uuid[16] = { 0xdd, 0xd3 };

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
	.output_size = 4,
	.output_actions = BT_MESH_DISPLAY_NUMBER,
	.output_number = output_number,
	.complete = prov_complete,
	.reset = prov_reset,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");


	err = bt_mesh_init(&prov, &comp);
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

#define ROTARY_NAME(id) _CONCAT(rotary_, id)
#define ROTARY_DEV(id)  (&ROTARY_NAME(id))
#define ROTARY_DEVICE_DEFINE(id, connector)                           \
				MINODE_ROTARY_DEVICE_DEFINE(ROTARY_NAME(id), _CONCAT(A, id),   \
				STRINGIFY(id),                                               \
				rotary_on_level_change)

void rotary_on_level_change(struct minode_rotary_device *dev,
        enum minode_rotary_level prev_level, enum minode_rotary_level new_level)
{
  const char *id = dev->user_data;
  printk("Light[%s] attached on %s level changed: prev_level=%d, new_level=%d.\n", id,
      dev->connector, prev_level, new_level);

	gen_level_set(&root_models[2], new_level * (65535/9) - 32768, 100, 1000);
}

ROTARY_DEVICE_DEFINE(0, A0);

void main(void)
{
	int err;

	printk("Dimmer Initializing...\n");

	level_cli_init();
	board_init();

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}

	minode_rotary_init(ROTARY_DEV(0));
	minode_rotary_start_listening(ROTARY_DEV(0));
}
