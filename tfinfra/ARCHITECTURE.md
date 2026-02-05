# Terraform Infrastructure Diagram

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         Google Cloud Project                         │
└─────────────────────────────────────────────────────────────────────┘
                                   │
                                   │
                    ┌──────────────▼──────────────┐
                    │   VPC Network: mynetwork    │
                    │   (Auto Mode)               │
                    │   - Subnetworks in all      │
                    │     regions automatically   │
                    └──────────────┬──────────────┘
                                   │
                    ┌──────────────▼──────────────┐
                    │   Firewall Rule             │
                    │   mynetwork-allow-http-     │
                    │   ssh-rdp-icmp              │
                    │                             │
                    │   Allows:                   │
                    │   • TCP 22 (SSH)            │
                    │   • TCP 80 (HTTP)           │
                    │   • TCP 3389 (RDP)          │
                    │   • ICMP (ping)             │
                    │   From: 0.0.0.0/0 (all)     │
                    └──────────────┬──────────────┘
                                   │
                    ┌──────────────┴──────────────┐
                    │                             │
         ┌──────────▼──────────┐      ┌──────────▼──────────┐
         │   VM Instance        │      │   VM Instance        │
         │   mynet-vm-1         │      │   mynet-vm-2         │
         │                      │      │                      │
         │   Zone:              │      │   Zone:              │
         │   us-east1-b         │      │   europe-west1-d     │
         │                      │      │                      │
         │   Machine Type:      │      │   Machine Type:      │
         │   e2-micro           │      │   e2-micro           │
         │                      │      │                      │
         │   OS:                │      │   OS:                │
         │   Debian 11          │      │   Debian 11          │
         │                      │      │                      │
         │   Network:           │      │   Network:           │
         │   mynetwork          │      │   mynetwork          │
         │                      │      │                      │
         │   Internal IP:       │      │   Internal IP:       │
         │   10.142.X.X         │      │   10.132.X.X         │
         │                      │      │                      │
         │   External IP:       │      │   External IP:       │
         │   Ephemeral          │      │   Ephemeral          │
         └──────────────────────┘      └──────────────────────┘
```

## Resource Dependencies

The Terraform configuration creates resources in this order:

```
1. google_compute_network (mynetwork)
   │
   ├─► 2. google_compute_firewall (mynetwork-allow-http-ssh-rdp-icmp)
   │
   ├─► 3. module.mynet-vm-1 (google_compute_instance)
   │
   └─► 4. module.mynet-vm-2 (google_compute_instance)
```

## Module Structure

```
tfinfra/
├── provider.tf                  # Defines Google Cloud as provider
│
├── mynetwork.tf                 # Main configuration file
│   ├── Creates VPC network
│   ├── Creates firewall rule
│   └── Calls instance module twice
│
└── instance/                    # Reusable VM module
    ├── main.tf                  # VM resource definition
    └── variables.tf             # Module inputs
        ├── instance_name
        ├── instance_zone
        ├── instance_network
        └── instance_type (default: e2-micro)
```

## Network Topology

```
┌────────────────────────────────────────────────────────────────┐
│                    Auto Mode VPC Network                        │
│                                                                 │
│  Region: us-east1               Region: europe-west1           │
│  ┌─────────────────────┐        ┌─────────────────────┐       │
│  │ Subnet: auto-subnet │        │ Subnet: auto-subnet │       │
│  │ IP Range: 10.142/20 │        │ IP Range: 10.132/20 │       │
│  │                     │        │                     │       │
│  │  ┌───────────────┐  │        │  ┌───────────────┐  │       │
│  │  │ mynet-vm-1    │  │        │  │ mynet-vm-2    │  │       │
│  │  │ 10.142.0.2    │◄─┼────────┼─►│ 10.132.0.2    │  │       │
│  │  └───────────────┘  │        │  └───────────────┘  │       │
│  └─────────────────────┘        └─────────────────────┘       │
│                                                                 │
│  + More auto-created subnets in other regions...               │
└────────────────────────────────────────────────────────────────┘
```

## Traffic Flow

### SSH Connection
```
Your Computer
     │
     │ SSH (TCP 22)
     ▼
[Firewall Rule: Allow TCP 22]
     │
     ▼
VM Instance (mynet-vm-1 or mynet-vm-2)
```

### Inter-VM Communication
```
mynet-vm-1 (us-east1-b)
     │
     │ Ping (ICMP)
     ▼
[Firewall Rule: Allow ICMP]
     │
     ▼
mynet-vm-2 (europe-west1-d)
```

### HTTP Traffic
```
Internet Client
     │
     │ HTTP (TCP 80)
     ▼
[Firewall Rule: Allow TCP 80]
     │
     ▼
VM Instance (External IP)
```

## Terraform Workflow

```
┌──────────────┐
│ Write Config │  ← provider.tf, mynetwork.tf, instance/*.tf
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ terraform    │  ← Downloads Google Cloud provider plugin
│ init         │     Initializes backend and modules
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ terraform    │  ← Formats code to canonical style
│ fmt          │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ terraform    │  ← Validates syntax and logic
│ validate     │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ terraform    │  ← Shows what will be created/changed
│ plan         │     without making changes
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ terraform    │  ← Creates actual resources in GCP
│ apply        │     Requires user confirmation
└──────┬───────┘
       │
       ▼
┌──────────────┐
│ Resources    │  ← VPC, Firewall, 2 VMs created
│ Created      │
└──────────────┘
```

## State Management

```
terraform.tfstate
├── google_compute_network.mynetwork
├── google_compute_firewall.mynetwork-allow-http-ssh-rdp-icmp
├── module.mynet-vm-1.google_compute_instance.vm_instance
└── module.mynet-vm-2.google_compute_instance.vm_instance
```

The state file tracks:
- Resource IDs
- Current configuration
- Resource attributes
- Dependencies

⚠️ **Important**: Never edit `terraform.tfstate` manually!

## Security Configuration

```
Firewall Rule: mynetwork-allow-http-ssh-rdp-icmp
├── Direction: Ingress (incoming traffic)
├── Priority: 1000 (default)
├── Action: Allow
├── Source: 0.0.0.0/0 (all IPs)
└── Rules:
    ├── Protocol: TCP, Ports: [22, 80, 3389]
    └── Protocol: ICMP (all codes)
```

**Note**: In production, restrict `source_ranges` to specific IPs!

## Cost Breakdown

```
┌────────────────────────────────────────────────────┐
│ Resource                      │ Estimated Cost/Month│
├────────────────────────────────────────────────────┤
│ mynet-vm-1 (e2-micro)         │ ~$6.11 USD         │
│ mynet-vm-2 (e2-micro)         │ ~$6.11 USD         │
│ External IP #1 (ephemeral)    │ ~$3.65 USD         │
│ External IP #2 (ephemeral)    │ ~$3.65 USD         │
│ VPC Network (mynetwork)       │ $0.00 USD          │
│ Firewall Rules                │ $0.00 USD          │
├────────────────────────────────────────────────────┤
│ TOTAL (approx.)               │ ~$19.52 USD        │
└────────────────────────────────────────────────────┘
```

*Prices are estimates for us-east1 region. Actual costs may vary by region and usage.*

## Quick Reference

| What | Where |
|------|-------|
| VPC Network | Console → VPC Networks → mynetwork |
| Firewall Rules | Console → VPC Firewall → mynetwork-allow-* |
| VM Instances | Console → Compute Engine → VM instances |
| SSH to VM | `gcloud compute ssh mynet-vm-1 --zone=us-east1-b` |
| View State | `terraform show` |
| List Resources | `terraform state list` |
| Destroy All | `terraform destroy` |

---

**Generated for**: Terraform Infrastructure Automation Lab  
**Last Updated**: February 2026
