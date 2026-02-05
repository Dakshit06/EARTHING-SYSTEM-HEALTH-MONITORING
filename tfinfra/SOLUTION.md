# 🚀 Terraform Deployment - Solution to "No Configuration Files" Error

## ❌ The Problem You're Facing

```bash
student@cloudshell:~ $ terraform apply
╷
│ Error: No configuration files
│ 
│ Apply requires configuration to be present.
╵
```

## ✅ The Solution (3 Simple Steps)

### Step 1: Navigate to the Terraform Directory
```bash
cd tfinfra
```

**Why?** All Terraform configuration files (`.tf` files) are in the `tfinfra` directory!

### Step 2: Initialize Terraform
```bash
terraform init
```

**Output you should see:**
```
Initializing modules...
Initializing the backend...
Initializing provider plugins...
- Finding latest version of hashicorp/google...
- Installing hashicorp/google...

Terraform has been successfully initialized!
```

### Step 3: Deploy the Infrastructure
```bash
terraform apply
```

**When prompted, type:** `yes`

---

## 📋 Complete Copy-Paste Solution

Just copy and paste these commands in order:

```bash
# Navigate to the correct directory
cd tfinfra

# Initialize Terraform (download plugins)
terraform init

# Preview what will be created (optional but recommended)
terraform plan

# Deploy the infrastructure
terraform apply
# Type 'yes' when prompted
```

---

## 🎯 What Gets Created

After running `terraform apply`, you'll have:

✅ **1 VPC Network** (`mynetwork`)  
✅ **1 Firewall Rule** (allows HTTP, SSH, RDP, ICMP)  
✅ **2 VM Instances**:
   - `mynet-vm-1` in US East (us-east1-b)
   - `mynet-vm-2` in Europe West (europe-west1-d)

---

## 🔍 Verify Your Deployment

### In Cloud Console:
1. **VPC Networks**: https://console.cloud.google.com/networking/networks/list
2. **Firewall Rules**: https://console.cloud.google.com/networking/firewalls/list  
3. **VM Instances**: https://console.cloud.google.com/compute/instances

### From Command Line:
```bash
# List all VMs
gcloud compute instances list

# SSH to VM 1
gcloud compute ssh mynet-vm-1 --zone=us-east1-b

# SSH to VM 2
gcloud compute ssh mynet-vm-2 --zone=europe-west1-d
```

---

## 🧪 Test Connectivity Between VMs

1. SSH into the first VM:
   ```bash
   gcloud compute ssh mynet-vm-1 --zone=us-east1-b
   ```

2. Get the internal IP of the second VM:
   ```bash
   gcloud compute instances describe mynet-vm-2 \
     --zone=europe-west1-d \
     --format='get(networkInterfaces[0].networkIP)'
   ```

3. Ping the second VM (replace with actual IP):
   ```bash
   ping -c 3 10.X.X.X
   ```

   **Expected result:** ✅ Ping should work!

---

## 🧹 Clean Up (Destroy Resources)

When you're done with the lab:

```bash
cd tfinfra
terraform destroy
# Type 'yes' when prompted
```

**This will delete all created resources and stop billing.**

---

## 💡 Quick Troubleshooting

| Problem | Solution |
|---------|----------|
| "No configuration files" | Run `cd tfinfra` first |
| "Terraform not initialized" | Run `terraform init` |
| "API not enabled" | Run `gcloud services enable compute.googleapis.com` |
| "Permission denied" | Check your GCP project permissions |

---

## 📚 More Information

- **Quick Start**: See `QUICKSTART.md`
- **Full Documentation**: See `README.md`
- **Architecture Diagrams**: See `ARCHITECTURE.md`
- **Automated Setup**: Run `./setup.sh`

---

## 🎓 Understanding the Lab

This Terraform configuration automates the deployment of:

```
Google Cloud Project
    └── VPC Network (mynetwork)
        ├── Firewall Rule (HTTP, SSH, RDP, ICMP)
        ├── VM Instance 1 (Debian 11, us-east1-b)
        └── VM Instance 2 (Debian 11, europe-west1-d)
```

**Key Concept**: Infrastructure as Code (IaC)
- Write configuration once
- Deploy consistently
- Version control your infrastructure
- Reproduce environments easily

---

## ✨ Success Checklist

After running `terraform apply`, check these:

- [ ] Output shows "Apply complete! Resources: 4 added"
- [ ] No error messages in terminal
- [ ] Can see `mynetwork` in GCP Console
- [ ] Can see 2 VMs in Compute Engine
- [ ] Can SSH into both VMs
- [ ] Ping works between VMs

---

## 🆘 Still Stuck?

1. **Check you're in the right directory:**
   ```bash
   pwd  # Should show: .../tfinfra
   ls   # Should show: provider.tf, mynetwork.tf, instance/
   ```

2. **Verify Terraform is installed:**
   ```bash
   terraform version
   ```

3. **Check GCP authentication:**
   ```bash
   gcloud auth list
   gcloud config get-value project
   ```

4. **Review the detailed guides:**
   - `QUICKSTART.md` - Step-by-step walkthrough
   - `README.md` - Complete reference
   - `ARCHITECTURE.md` - Visual diagrams

---

**🎉 You've got this! The solution is just three commands away:**

```bash
cd tfinfra
terraform init
terraform apply
```

---

*Lab: Automating the Deployment of Infrastructure Using Terraform*  
*Project: Earthing System Health Monitoring*  
*Last Updated: February 2026*
