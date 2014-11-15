#ifndef SG17_SYSFS_H
#define SG17_SYSFS_H
#include <linux/kernel.h>
#include <linux/config.h>
#include <linux/kobject.h>
#include <linux/netdevice.h>

/* --------------------------------------------------------------------------
 *      Module initialisation/cleanup
 * -------------------------------------------------------------------------- */

#define to_net_dev(class) container_of(class, struct net_device, class_dev)

int sg17_sysfs_register(struct net_device *ndev);
void sg17_sysfs_remove(struct net_device *ndev);

#endif
