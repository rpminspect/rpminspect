/*
 * derp kernel module
 * Copyright (C) 2020 David Cantrell <dcantrell@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_AUTHOR("David Cantrell <dcantrell@redhat.com>");
MODULE_DESCRIPTION("derp testing module");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
MODULE_INFO(derp, "derp");

/* For the aliases check, leave this here to match alias wildcards */
MODULE_ALIAS("pci:v00001425d00000020sv*sd*bc*sc*i*");

static int derp_count = 1;
static char *derp_text = "derp";

#ifdef _USE_MODULE_PARAMETERS
module_param(derp_count, int, 0660);
module_param(derp_text, charp, 0660);
#endif

static int derp_proc_show(struct seq_file *m, void *v)
{
    int i;

    for (i = 0; i < derp_count; i++) {
        seq_printf(m, "%s\n", derp_text);
    }

    return 0;
}

static int derp_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, derp_proc_show, NULL);
}

static const struct file_operations derp_proc_fops = {
    .owner   = THIS_MODULE,
    .open    = derp_proc_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int __init derp_init(void)
{
    proc_create("derp", 0, NULL, &derp_proc_fops);
    printk(KERN_INFO "derp activated\n");
    return 0;
}

static void __exit derp_exit(void)
{
    remove_proc_entry("derp", NULL);
    printk(KERN_INFO "derp deactivated\n");
}

module_init(derp_init);
module_exit(derp_exit);

#ifdef _USE_MODULE_DEPENDS
MODULE_SOFTDEP("pre: video");
#endif

#ifdef _USE_MODULE_ALIASES
/* Just some text */
MODULE_ALIAS("pci:lorem*ipsum");

/* PCI ID wildcard stuff */
MODULE_ALIAS("pci:v00001425d00000020sv*sd00000001bc*sc*i*");
#endif
