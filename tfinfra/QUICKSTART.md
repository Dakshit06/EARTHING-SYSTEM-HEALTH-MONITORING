# Terraform Quick Start Guide

## Problem: "No configuration files" Error

If you see this error:
```
Error: No configuration files

Apply requires configuration to be present.
```

**Solution**: You need to navigate to the `tfinfra` directory first!

## Step-by-Step Instructions

### 1. Navigate to the tfinfra Directory

The Terraform configuration files are located in the `tfinfra` folder. You must run all Terraform commands from inside this directory.

```bash
cd tfinfra
```

### 2. Verify You're in the Right Directory

Check that you can see the Terraform files:

```bash
ls -la
```

You should see:
- `provider.tf`
- `mynetwork.tf`
- `instance/` directory
- `README.md`

### 3. Initialize Terraform (First Time Only)

```bash
terraform init
```

Expected output:
```
Initializing modules...
Initializing the backend...
Initializing provider plugins...
...
Terraform has been successfully initialized!
```

### 4. Format the Configuration (Optional)

```bash
terraform fmt
```

### 5. Validate the Configuration

```bash
terraform validate
```

Expected output:
```
Success! The configuration is valid.
```

### 6. Preview the Changes

```bash
terraform plan
```

This shows you what resources will be created without actually creating them.

### 7. Apply the Configuration

```bash
terraform apply
```

When prompted, type `yes` to confirm.

Expected output:
```
Plan: 4 to add, 0 to change, 0 to destroy.

Do you want to perform these actions?
  Terraform will perform the actions described above.
  Only 'yes' will be accepted to approve.

  Enter a value: yes
```

## Complete Command Sequence

Here's the complete sequence of commands to copy and paste:

```bash
# Step 1: Navigate to the tfinfra directory
cd tfinfra

# Step 2: Initialize Terraform (first time only)
terraform init

# Step 3: Format configuration files
terraform fmt

# Step 4: Validate the configuration
terraform validate

# Step 5: Preview changes
terraform plan

# Step 6: Deploy infrastructure
terraform apply
# Type 'yes' when prompted
```

## Using the Setup Script (Alternative)

We've provided a convenient setup script that automates steps 2-5:

```bash
cd tfinfra
./setup.sh
```

Then run `terraform apply` to deploy.

## Common Mistakes

### ❌ Wrong: Running from the wrong directory
```bash
student@cloudshell:~ $ terraform apply
Error: No configuration files
```

### ✅ Correct: Running from the tfinfra directory
```bash
student@cloudshell:~ $ cd tfinfra
student@cloudshell:~/tfinfra $ terraform apply
```

### ❌ Wrong: Forgetting to initialize
```bash
student@cloudshell:~/tfinfra $ terraform apply
Error: Terraform not initialized
```

### ✅ Correct: Initialize first
```bash
student@cloudshell:~/tfinfra $ terraform init
student@cloudshell:~/tfinfra $ terraform apply
```

## Visual Directory Structure

```
Your Project Root/
└── tfinfra/              ← You must be HERE
    ├── provider.tf
    ├── mynetwork.tf
    ├── setup.sh
    ├── README.md
    ├── QUICKSTART.md     ← This file
    └── instance/
        ├── main.tf
        └── variables.tf
```

## Troubleshooting

### "Error: No configuration files"

**Cause**: You're in the wrong directory.

**Solution**: 
```bash
pwd  # Check current directory
cd tfinfra  # Navigate to tfinfra directory
terraform apply  # Try again
```

### "Error: Could not load plugin"

**Cause**: Terraform needs to be initialized.

**Solution**:
```bash
terraform init
terraform apply
```

### "Error: Missing required argument"

**Cause**: Configuration file might be corrupted or incomplete.

**Solution**:
```bash
terraform validate  # Check for errors
cat provider.tf     # Verify file exists and has content
cat mynetwork.tf    # Verify file exists and has content
```

## Verifying Deployment

After successful deployment, verify in Google Cloud Console:

1. **VPC Networks**: 
   ```
   https://console.cloud.google.com/networking/networks/list
   ```
   Look for: `mynetwork`

2. **Firewall Rules**:
   ```
   https://console.cloud.google.com/networking/firewalls/list
   ```
   Look for: `mynetwork-allow-http-ssh-rdp-icmp`

3. **VM Instances**:
   ```
   https://console.cloud.google.com/compute/instances
   ```
   Look for: `mynet-vm-1` and `mynet-vm-2`

## Testing the Deployment

SSH into the first VM:
```bash
gcloud compute ssh mynet-vm-1 --zone=us-east1-b
```

From inside mynet-vm-1, ping the second VM:
```bash
# Get the internal IP of mynet-vm-2 first
gcloud compute instances describe mynet-vm-2 --zone=europe-west1-d --format='get(networkInterfaces[0].networkIP)'

# Then ping it (replace with actual IP)
ping -c 3 10.X.X.X
```

## Cleaning Up

To destroy all created resources:

```bash
cd tfinfra
terraform destroy
# Type 'yes' when prompted
```

## Need More Help?

- Full documentation: See `README.md` in the tfinfra directory
- Terraform docs: https://www.terraform.io/docs
- Google Cloud Provider: https://registry.terraform.io/providers/hashicorp/google/latest/docs

## Summary Cheat Sheet

| Command | Purpose |
|---------|---------|
| `cd tfinfra` | Navigate to Terraform directory (REQUIRED) |
| `terraform init` | Initialize Terraform (first time only) |
| `terraform fmt` | Format configuration files |
| `terraform validate` | Check configuration syntax |
| `terraform plan` | Preview changes |
| `terraform apply` | Create infrastructure |
| `terraform show` | Show current state |
| `terraform destroy` | Delete all resources |

**Remember**: Always run Terraform commands from inside the `tfinfra` directory!
