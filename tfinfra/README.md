# Terraform Infrastructure Automation

This directory contains Terraform configuration files to automate the deployment of Google Cloud infrastructure for the Earthing System Health Monitoring project.

## Overview

This Terraform configuration deploys:
- **1 Auto mode VPC network** (`mynetwork`) with subnetworks in every region
- **1 Firewall rule** (`mynetwork-allow-http-ssh-rdp-icmp`) allowing HTTP, SSH, RDP, and ICMP traffic
- **2 VM instances**:
  - `mynet-vm-1` in `us-east1-b` zone
  - `mynet-vm-2` in `europe-west1-d` zone

## Architecture

```
mynetwork (Auto mode VPC)
├── Firewall Rule: mynetwork-allow-http-ssh-rdp-icmp
│   ├── TCP ports: 22 (SSH), 80 (HTTP), 3389 (RDP)
│   └── ICMP protocol
├── VM Instance: mynet-vm-1 (us-east1-b)
│   ├── Machine Type: e2-micro
│   ├── OS: Debian 11
│   └── External IP: Ephemeral
└── VM Instance: mynet-vm-2 (europe-west1-d)
    ├── Machine Type: e2-micro
    ├── OS: Debian 11
    └── External IP: Ephemeral
```

## Directory Structure

```
tfinfra/
├── provider.tf          # Google Cloud provider configuration
├── mynetwork.tf         # Network, firewall, and VM instance definitions
├── instance/            # Reusable VM instance module
│   ├── main.tf         # VM instance resource definition
│   └── variables.tf    # Input variables for the module
└── README.md           # This file
```

## Prerequisites

Before you begin, ensure you have:

1. **Terraform** installed (v1.5.7 or later)
   ```bash
   terraform --version
   ```

2. **Google Cloud SDK** installed and configured
   ```bash
   gcloud --version
   ```

3. **Google Cloud Project** with:
   - Compute Engine API enabled
   - Appropriate permissions (Compute Admin or Project Editor)
   - Application Default Credentials configured

## Setup Instructions

### 1. Configure Google Cloud Credentials

Set up authentication with Google Cloud:

```bash
# Login to Google Cloud
gcloud auth login

# Set your project ID
gcloud config set project YOUR_PROJECT_ID

# Configure application default credentials
gcloud auth application-default login
```

### 2. Initialize Terraform

Navigate to the `tfinfra` directory and initialize Terraform:

```bash
cd tfinfra
terraform init
```

This command downloads the Google Cloud provider plugin and sets up the backend.

### 3. Format Configuration Files

Ensure all Terraform files are properly formatted:

```bash
terraform fmt
```

### 4. Validate Configuration

Validate the syntax and configuration:

```bash
terraform validate
```

### 5. Preview Changes

Generate an execution plan to see what resources will be created:

```bash
terraform plan
```

Expected output:
```
Plan: 4 to add, 0 to change, 0 to destroy.
```

### 6. Deploy Infrastructure

Apply the configuration to create the resources:

```bash
terraform apply
```

Type `yes` when prompted to confirm.

### 7. Verify Deployment

After successful deployment, verify the resources in Google Cloud Console:

- **VPC Networks**: https://console.cloud.google.com/networking/networks/list
- **Firewall Rules**: https://console.cloud.google.com/networking/firewalls/list
- **VM Instances**: https://console.cloud.google.com/compute/instances

## Usage

### Connect to VM Instances

Once deployed, you can SSH into the VM instances:

```bash
# SSH to mynet-vm-1
gcloud compute ssh mynet-vm-1 --zone=us-east1-b

# SSH to mynet-vm-2
gcloud compute ssh mynet-vm-2 --zone=europe-west1-d
```

### Test Connectivity

From `mynet-vm-1`, ping `mynet-vm-2` using its internal IP:

```bash
ping -c 3 INTERNAL_IP_OF_MYNET_VM_2
```

### View Outputs

To display the current state of resources:

```bash
terraform show
```

## Configuration Variables

The instance module accepts the following variables:

| Variable | Description | Required | Default |
|----------|-------------|----------|---------|
| `instance_name` | Name of the VM instance | Yes | - |
| `instance_zone` | GCP zone for the instance | Yes | - |
| `instance_network` | VPC network for the instance | Yes | - |
| `instance_type` | Machine type | No | `e2-micro` |

## Customization

### Change VM Instance Zones

Edit `mynetwork.tf` and modify the `instance_zone` parameter:

```hcl
module "mynet-vm-1" {
  source           = "./instance"
  instance_name    = "mynet-vm-1"
  instance_zone    = "us-central1-a"  # Changed zone
  instance_network = google_compute_network.mynetwork.self_link
}
```

### Change Machine Type

Modify the `instance_type` in the module call:

```hcl
module "mynet-vm-1" {
  source           = "./instance"
  instance_name    = "mynet-vm-1"
  instance_zone    = "us-east1-b"
  instance_network = google_compute_network.mynetwork.self_link
  instance_type    = "e2-small"  # Larger machine type
}
```

### Add More Firewall Rules

Add additional `google_compute_firewall` resources to `mynetwork.tf`:

```hcl
resource "google_compute_firewall" "mynetwork-allow-custom" {
  name    = "mynetwork-allow-custom"
  network = google_compute_network.mynetwork.self_link

  allow {
    protocol = "tcp"
    ports    = ["8080", "8443"]
  }

  source_ranges = ["0.0.0.0/0"]
}
```

## Destroying Resources

To remove all created resources:

```bash
terraform destroy
```

Type `yes` when prompted to confirm the destruction.

⚠️ **Warning**: This will permanently delete all resources created by this configuration.

## Best Practices

1. **State Management**: Keep your `terraform.tfstate` file secure and backed up
2. **Version Control**: Commit your `.tf` files but exclude `terraform.tfstate`
3. **Remote Backend**: Consider using Google Cloud Storage for remote state storage
4. **Variables**: Use `terraform.tfvars` for environment-specific values
5. **Modules**: Leverage the module pattern for reusable infrastructure components

## Troubleshooting

### Authentication Issues

If you encounter authentication errors:

```bash
# Re-authenticate
gcloud auth application-default login

# Verify credentials
gcloud auth list
```

### API Not Enabled

If you get "API not enabled" errors:

```bash
# Enable Compute Engine API
gcloud services enable compute.googleapis.com
```

### Permission Denied

Ensure your user account has the necessary IAM roles:

```bash
# List current IAM policy
gcloud projects get-iam-policy YOUR_PROJECT_ID
```

Required roles:
- `roles/compute.admin` or
- `roles/editor` or
- `roles/owner`

## Resources

- [Terraform Documentation](https://www.terraform.io/docs)
- [Google Cloud Provider Documentation](https://registry.terraform.io/providers/hashicorp/google/latest/docs)
- [Compute Engine Documentation](https://cloud.google.com/compute/docs)
- [VPC Network Documentation](https://cloud.google.com/vpc/docs)

## Cost Considerations

The deployed resources incur Google Cloud costs:

- **e2-micro instances**: ~$6.11/month each (if running 24/7)
- **VPC network**: No charge
- **Firewall rules**: No charge
- **External IP addresses**: ~$3.65/month each

**Estimated monthly cost**: ~$20-25 for 2 e2-micro instances

Use the [Google Cloud Pricing Calculator](https://cloud.google.com/products/calculator) for accurate estimates.

## Support

For issues related to:
- **Terraform configuration**: Check the [Terraform documentation](https://www.terraform.io/docs)
- **Google Cloud Platform**: Visit [GCP support](https://cloud.google.com/support)
- **This project**: Open an issue on the [GitHub repository](https://github.com/Dakshit06/EARTHING-SYSTEM-HEALTH-MONITORING/issues)

## License

This Terraform configuration is part of the Earthing System Health Monitoring project and is licensed under the MIT License.

---

**Created for**: Automating the Deployment of Infrastructure Using Terraform Lab  
**Last Updated**: February 2026
