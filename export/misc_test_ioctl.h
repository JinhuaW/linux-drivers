#ifndef __MISC_TEST_IOCTL_H__
#define __MISC_TEST_IOCTL_H__

#define MISC_TEST_IOC_MAGIC 'X'
#define MISC_TEST_SET_PCIE_MAPPING      _IOW(MISC_TEST_IOC_MAGIC, 0, unsigned long long)
#define MISC_TEST_SET_SLOT      _IOW(MISC_TEST_IOC_MAGIC, 1, unsigned long long)

#define MISC_MAX_SLOT 6
#define MISC_MAX_BAY 5
#define MISC_MAX_PORT (MISC_MAX_SLOT * MISC_MAX_BAY) 

typedef struct switch_port {
	unsigned long long slot;
	unsigned long long bay;
} switch_port_t;

typedef struct pci_port {
	unsigned long long sec_bus;
	unsigned long long sub_bus;
	unsigned long long slot;
	unsigned long long bay;
} pci_port_t;

typedef struct pci_array {
	unsigned long long num;
	unsigned long long ents;
} pci_array_t;
#endif
