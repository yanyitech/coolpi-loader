/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define	DEBUG	*/

#include <common.h>
#include <autoboot.h>
#include <cli.h>
#include <console.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

#include <command.h>
#include <mmc.h>

extern void fes_hub_rst(void);
extern void dir_switch_sdio(void);

static int do_load_version(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint32_t vlen = 0;
	char cmd_mod[128] = {0};

	rockchip_show_logo();
	run_command("run distro_bootcmd;", -1);
	printf("Loading order: usb - tf - emmc\n");
	fes_hub_rst();
	run_command("usb reset;", -1);
	mdelay(1000);
	run_command("checkconf usb;", -1);
	if(!run_command("load usb 0:1 ${loadaddr_} ${bootdir}${image}", 0)) {
		run_command("load usb 0:1 ${initrd_addr} ${bootdir}${rd_file}", -1);
		vlen = simple_strtoul(env_get("filesize"), NULL, 16);
		run_command("load usb 0:1 ${fdt_addr_r} ${bootdir}${fdt_file}", -1);
		dir_switch_sdio();
		sprintf(cmd_mod, "unzip ${loadaddr_} ${loadaddr};booti ${loadaddr} ${initrd_addr}:%x ${fdt_addr_r}", vlen);
		run_command(cmd_mod, -1);
		return 0;
	}
	run_command("checkconf tf;", -1);
	if(!run_command("load mmc 1:1 ${loadaddr_} ${bootdir}${image}", 0)) {
		run_command("load mmc 1:1 ${initrd_addr} ${bootdir}${rd_file}", -1);
		vlen = simple_strtoul(env_get("filesize"), NULL, 16);
		run_command("load mmc 1:1 ${fdt_addr_r} ${bootdir}${fdt_file}", -1);
		dir_switch_sdio();
		sprintf(cmd_mod, "unzip ${loadaddr_} ${loadaddr};booti ${loadaddr} ${initrd_addr}:%x ${fdt_addr_r}", vlen);
		run_command(cmd_mod, -1);
		return 0;
	}

	run_command("checkconf mmc;", -1);
	if(!run_command("load mmc 0:1 ${loadaddr_} ${bootdir}${image}", 0)) {
		run_command("load mmc 0:1 ${initrd_addr} ${bootdir}${rd_file}", -1);
		vlen = simple_strtoul(env_get("filesize"), NULL, 16);
		run_command("load mmc 0:1 ${fdt_addr_r} ${bootdir}${fdt_file}", -1);
		dir_switch_sdio();
		sprintf(cmd_mod, "unzip ${loadaddr_} ${loadaddr};booti ${loadaddr} ${initrd_addr}:%x ${fdt_addr_r}", vlen);
		run_command(cmd_mod, -1);
		return 0;
	}

	mdelay(3000);

        return 0;
}

U_BOOT_CMD(loadver, 4, 0, do_load_version,
           "Load version",
           "Booting\n"
);

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

static void run_preboot_environment_command(void)
{
#ifdef CONFIG_PREBOOT
	char *p;

	p = env_get("preboot");
	if (p != NULL) {
# ifdef CONFIG_AUTOBOOT_KEYED
		int prev = disable_ctrlc(1);	/* disable Control C checking */
# endif

		run_command_list(p, -1, 0);

# ifdef CONFIG_AUTOBOOT_KEYED
		disable_ctrlc(prev);	/* restore Control C checking */
# endif
	}
#endif /* CONFIG_PREBOOT */
}

/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;
	char *temp = NULL;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

#ifdef CONFIG_VERSION_VARIABLE
	env_set("ver", version_string);  /* set version variable */
#endif /* CONFIG_VERSION_VARIABLE */

	cli_init();

	run_preboot_environment_command();


#if defined(CONFIG_UPDATE_TFTP)
	update_tftp(0UL, NULL, NULL);
#endif /* CONFIG_UPDATE_TFTP */

	run_command("c read;", -1);

	env_set("board_name", "CoolPi RK3588(S)");
	env_set("vendor", "SZ YanYi TECH");

	temp = env_get("fixmac");
	if(temp) {
		if(strncmp(env_get("fixmac"), "yes", 3)) {
			env_set("fixmac", "yes");
			run_command("c write;", -1);
		}
	} else {
		env_set("fixmac", "yes");
		run_command("c write;", -1);
	}
	run_command("c read;", -1);
	env_set("board", "coolpi");
	env_set("board_name", "CoolPi RK3588(S)");
	env_set("vendor", "SZ YanYi TECH");
	env_set("soc", "rk3588(s)");
	env_set("eth1addr", "00:11:22:33:44:55");

	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	//show_loading_info();
	autoboot_command(s);

	cli_loop();
	panic("No CLI available");
}
