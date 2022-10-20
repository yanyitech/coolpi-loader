/*
 * (C) Copyright jack@cool-pi.com
 */

#include <common.h>
#include <command.h>
#include <usb.h>
#include <mmc.h>
#include <fs.h>
#include <ext4fs.h>
#include <mtd.h>

#define MTD_SPINOR_NAME "nor0"

// Error code
enum udpate_ret_t {
	UPDATE_SUCCESS,     // 0 = Success
	UPDATE_FAIL,        // 1 = Failure
};

// Load type
enum file_load__t {
	USB_LOAD,
	NET_LOAD,
	EMMC_LOAD,
	TF_LOAD,
	RAM_LOAD,
};

// Install file type
enum file_install__t {
	BOOT_IMG,
	ROOTFS_IMG,
};

static int mmc_default_select(unsigned int dev)
{
	char cmd_temp[40];

	memset(cmd_temp, 0, sizeof(cmd_temp));
	sprintf(cmd_temp, "mmc dev %d", dev);
	debug("cmd: %s\n",cmd_temp);
	if(run_command(cmd_temp, 0)) {
		return UPDATE_FAIL;
	}

	return  UPDATE_SUCCESS;
}

static int net_load_file(char *filename, unsigned long addr, int *len)
{
	char cmd_temp[64];
	char *len_tmp = NULL;

	memset(cmd_temp, 0, sizeof(cmd_temp));
	if(strncmp(env_get("ip_dyn"), "yes", 3) == 0)
		sprintf(cmd_temp, "dhcp %x %s", (unsigned int)addr, filename);
	else
		sprintf(cmd_temp, "tftp %x %s", (unsigned int)addr, filename);
	//printf(" %s\n",cmd_temp);
	if(run_command(cmd_temp, 0)) {
		return UPDATE_FAIL;
	}

	len_tmp = env_get("filesize");
	if(len_tmp == NULL)
		return UPDATE_FAIL;
	*len = simple_strtoul(len_tmp, NULL, 16);
	printf("Net Got filesize 0x%x bytes\n", *len);

	return  UPDATE_SUCCESS;
}

static int usb_load_file(char *filename, unsigned long addr, int *len)
{
	if (fs_set_blk_dev("usb", "0", FS_TYPE_ANY))
		return UPDATE_FAIL;

	if(fs_read(filename, addr, 0, 0, (loff_t *)len) < 0 )
		return UPDATE_FAIL;
	//printf("Usb Got filesize 0x%x bytes\n", *len);

	return  UPDATE_SUCCESS;
}

int tf_load_file(char *filename, unsigned long addr, int size, int *len)
{
	mmc_default_select(1);
	if (fs_set_blk_dev("mmc", "1:1", FS_TYPE_ANY))
		return UPDATE_FAIL;
	if(fs_read(filename, addr, 0, size, (loff_t *)len) < 0 )
		return UPDATE_FAIL;
	//printf("emmc Got filesize 0x%x bytes\n", *len);

	return 0;
}

int emmc_load_file(char *filename, unsigned long addr, int size, int *len)
{
	mmc_default_select(0);
	if (fs_set_blk_dev("mmc", "0:1", FS_TYPE_ANY))
		return UPDATE_FAIL;
	if(fs_read(filename, addr, 0, size, (loff_t *)len) < 0 )
		return UPDATE_FAIL;
	//printf("emmc Got filesize 0x%x bytes\n", *len);

	return 0;
}

static int load_file_to_ram(int type, char *filename, unsigned long addr, int *file_size)
{
	if(type == USB_LOAD) {
		if(usb_load_file(filename, addr, file_size) == UPDATE_FAIL) {
			//printf("Usb load fail\n");
			return UPDATE_FAIL;
		}
	} else if(type == NET_LOAD) {
		if(net_load_file(filename, addr, file_size) == UPDATE_FAIL) {
			//printf("Net load fail\n");
			return UPDATE_FAIL;
		}
	} else if(type == EMMC_LOAD) {
		if(emmc_load_file(filename, addr, 0, file_size) == UPDATE_FAIL) {
			//printf("Emmc load fail\n");
			return UPDATE_FAIL;
		}
	} else if(type == TF_LOAD) {
		if(tf_load_file(filename, addr, 0, file_size) == UPDATE_FAIL) {
			//printf("TF load fail\n");
			return UPDATE_FAIL;
		}
	}
	
	return UPDATE_SUCCESS;
}


static int get_env_from_nor(void)
{
	char *mtd_name = MTD_SPINOR_NAME;
        struct mtd_info *mtd;
        size_t retlen, off;
        size_t size = 0;
        u_char *des_buf;
        int ret = 0;

        mtd = get_mtd_device_nm(mtd_name);
        if (IS_ERR_OR_NULL(mtd)) {
		printf("MTD device %s not found, ret %ld\n", mtd_name, PTR_ERR(mtd));
	        return CMD_RET_FAILURE;
	}

        des_buf = (u_char *)0x300000;
        off = 0x700000;
        size = 0x1000;
        ret = mtd_read(mtd, off, size, &retlen, des_buf);
        if (ret || size != retlen) {
                printf("mtd read fail, ret=%d retlen=%ld size=%ld\n", ret, retlen, size);
                return -1;
        }
	//printf("%s\n", des_buf);
	if(*des_buf != 0xff) {
		if (run_command("env import 0x300000", 0)) {
	        	return CMD_RET_FAILURE;
		}
	}

	return ret;
}

static int set_env_to_nor(void)
{
	char *mtd_name = MTD_SPINOR_NAME;
        struct mtd_info *mtd;
        size_t retlen, off;
        size_t size = 0;
        u_char *des_buf;
        int ret = 0;
	struct erase_info erase_op = {};

	if (run_command("env export 0x300000", 0)) {
	        return CMD_RET_FAILURE;
	}

        mtd = get_mtd_device_nm(mtd_name);
        if (IS_ERR_OR_NULL(mtd)) {
		printf("MTD device %s not found, ret %ld\n", mtd_name, PTR_ERR(mtd));
	        return CMD_RET_FAILURE;
	}

        des_buf = (u_char *)0x300000;
        off = 0x700000;
        size = 0x1000;

	erase_op.mtd = mtd;
	erase_op.addr = off;
	erase_op.len = size;
	mtd_erase(mtd, &erase_op);

        ret = mtd_write(mtd, off, size, &retlen, des_buf);
        if (ret || size != retlen) {
                printf("mtd write fail, ret=%d retlen=%ld size=%ld\n", ret, retlen, size);
                return -1;
        }

	return ret;
}

static int clr_env_to_nor(void)
{
	char *mtd_name = MTD_SPINOR_NAME;
        struct mtd_info *mtd;
        size_t off;
        size_t size = 0;
        int ret = 0;
	struct erase_info erase_op = {};

        mtd = get_mtd_device_nm(mtd_name);
        if (IS_ERR_OR_NULL(mtd)) {
		printf("MTD device %s not found, ret %ld\n", mtd_name, PTR_ERR(mtd));
	        return CMD_RET_FAILURE;
	}

        off = 0x700000;
        size = 0x1000;

	erase_op.mtd = mtd;
	erase_op.addr = off;
	erase_op.len = size;
	mtd_erase(mtd, &erase_op);

	run_command("env default -f -a", 0);
	set_env_to_nor();

	return ret;
}

static int upgrade_uboot(char *from)
{
	char *mtd_name = MTD_SPINOR_NAME;
        struct mtd_info *mtd;
        size_t retlen, off;
        size_t size = 0;
        u_char *des_buf;
        int ret = 0;
	struct erase_info erase_op = {};

	if (strcmp(from, "usb") == 0) {
		run_command("usb reset", 0);
		if(run_command("load usb 0:1 0x800000 uboot.img", 0)) {
	        	return CMD_RET_FAILURE;
		}
	} else if (strcmp(from, "mmc") == 0) {
		if(run_command("load mmc 0:1 0x800000 uboot.img", 0)) {
	        	return CMD_RET_FAILURE;
		}
	} else if (strcmp(from, "tf") == 0) {
		if(run_command("load mmc 1:1 0x800000 uboot.img", 0)) {
	        	return CMD_RET_FAILURE;
		}
	} else {
		ret = CMD_RET_USAGE;
		return ret;
	}

        mtd = get_mtd_device_nm(mtd_name);
        if (IS_ERR_OR_NULL(mtd)) {
		printf("MTD device %s not found, ret %ld\n", mtd_name, PTR_ERR(mtd));
	        return CMD_RET_FAILURE;
	}

        des_buf = (u_char *)0x800000;
        off = 0x200000;
        size = 0x500000;

	printf("wait for erase...\n");
	erase_op.mtd = mtd;
	erase_op.addr = off;
	erase_op.len = size;
	mtd_erase(mtd, &erase_op);

        ret = mtd_write(mtd, off, size, &retlen, des_buf);
        if (ret || size != retlen) {
                printf("mtd write fail, ret=%d retlen=%ld size=%ld\n", ret, retlen, size);
                return -1;
        }
	printf("uboot upgrade ok...\n");

	return ret;
}


static int do_nor_ops(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "read") == 0) {
		get_env_from_nor();
	} else if (strcmp(argv[1], "write") == 0) {
		set_env_to_nor();
	} else if (strcmp(argv[1], "update") == 0) {
		if (argc == 3)
			upgrade_uboot(argv[2]);
	} else if (strcmp(argv[1], "envclr") == 0) {
		clr_env_to_nor();
	} else {
		return CMD_RET_USAGE;
	}

	return 0;
}

static int do_checkconf(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	int type;
	int len;
	char str_temp[512];
	char str_env[512];
	char *p = NULL;
	int i = 0;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strcmp(argv[1], "usb") == 0) {
		type = USB_LOAD;
		//run_command("usb reset", 0);
	} else if (strcmp(argv[1], "mmc") == 0) {
		type = EMMC_LOAD;
	} else if (strcmp(argv[1], "tf") == 0) {
		type = TF_LOAD;
	} else {
		ret = CMD_RET_USAGE;
		return ret;
	}

	memset(str_temp, 0, sizeof(str_temp));
	if (load_file_to_ram(type, "cmdline.txt", (unsigned long) &str_temp[0], &len) != UPDATE_SUCCESS) {
		debug("Unable to read file %s\n", "cmdline.txt");
		return UPDATE_FAIL;
	}
	//printf("\n%s", str_temp);
	sprintf(str_env, "%s", str_temp);
	env_set("bootargs", str_env);

	memset(str_temp, 0, sizeof(str_temp));
	sprintf(str_temp, "rtleth=ethaddr:%s", env_get("ethaddr"));

	env_update("bootargs", str_temp);

	memset(str_temp, 0, sizeof(str_temp));
	if (load_file_to_ram(type, "config.txt", (unsigned long) &str_temp[0], &len) != UPDATE_SUCCESS) {
		debug("Unable to read file %s\n", "config.txt");
		return UPDATE_FAIL;
	}
	//printf("\n%s", str_temp);
	if(len < 24)
		return -1;

	memset(str_env, 0, sizeof(str_env));
	p = &str_temp[0];
	if(0 == strncmp(p, "bootdir=", 8)) {
		p += 8;
		//printf("first: %s\n", p);
		for(i = 0; i < (len - 8); i++) {
			if(p[i] == '\n') {
				i++;
				p += i;
				//printf("get %d byte\n", i);
				break;
			}
			str_env[i] = p[i];
		}
		str_env[i] = '\0';
		env_set("bootdir", str_env);
		//printf("second: %s\n", p);
		if(0 == strncmp(p, "kernel=", 7)) {
			p += 7;
			memset(str_env, 0, sizeof(str_env));
			for(i = 0; i < (len - 16); i++) {
				if(p[i] == '\n') {
					i++;
					p += i;
					//printf("get %d byte\n", i);
					break;
				}
				str_env[i] = p[i];
			}
			str_env[i] = '\0';
			env_set("image", str_env);
			//printf("third: %s\n", p);
			if(0 == strncmp(p, "initrd=", 7)) {
				p += 7;
				memset(str_env, 0, sizeof(str_env));
				for(i = 0; i < (len - 24); i++) {
					if(p[i] == '\n') {
						i++;
						p += i;
						//printf("get %d byte\n", i);
						break;
					}
					str_env[i] = p[i];
				}
				str_env[i] = '\0';
				env_set("rd_file", str_env);
				//printf("four: %s\n", p);
				if(0 == strncmp(p, "dtb=", 4)) {
					p += 4;
					memset(str_env, 0, sizeof(str_env));
					for(i = 0; i < (len - 24); i++) {
						if(p[i] == '\n') {
							i++;
							p += i;
							//printf("get %d byte\n", i);
							break;
						}
						str_env[i] = p[i];
					}
					str_env[i] = '\0';
					env_set("fdt_file", str_env);
				}
			}
		}
	}

	return ret;
}

U_BOOT_CMD(checkconf, 2, 0, do_checkconf,
           "",
           "\n"
);

U_BOOT_CMD(c, 3, 0, do_nor_ops,
           "",
           "\n"
);
