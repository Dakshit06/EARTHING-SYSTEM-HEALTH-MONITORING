# 🔧 STEP-BY-STEP SOLUTION: Fix "No configuration files" Error

## Your Current Problem

```bash
student@cloudshell:~ $ terraform apply
╷
│ Error: No configuration files
│ 
│ Apply requires configuration to be present.
╵
```

---

## ⚠️ The Issue

You're running `terraform apply` from your **home directory** (`~`), but the Terraform configuration files are in a **subfolder** called `tfinfra`.

Terraform can only see `.tf` files in the **current directory**, so it can't find anything!

---

## ✅ The Solution (Copy & Paste These Commands)

### STEP 1: Go to the Terraform Directory

```bash
cd tfinfra
```

**What this does:** Changes your current directory to where the Terraform files are located.

**Verify it worked:**
```bash
pwd
# Should show: .../EARTHING-SYSTEM-HEALTH-MONITORING/tfinfra

ls *.tf
# Should show: mynetwork.tf  provider.tf
```

---

### STEP 2: Initialize Terraform

```bash
terraform init
```

**What this does:** Downloads the Google Cloud provider plugin that Terraform needs to work with GCP.

**Expected output:**
```
Initializing modules...
Initializing the backend...
Initializing provider plugins...
- Finding latest version of hashicorp/google...
- Installing hashicorp/google...

Terraform has been successfully initialized!
```

✅ **Wait for this to complete before moving to Step 3!**

---

### STEP 3: Preview What Will Be Created (Optional but Recommended)

```bash
terraform plan
```

**What this does:** Shows you what resources Terraform will create **WITHOUT actually creating them**.

**You should see:**
```
Plan: 4 to add, 0 to change, 0 to destroy.
```

This means Terraform will create 4 resources:
1. A VPC network
2. A firewall rule
3. VM instance #1
4. VM instance #2

---

### STEP 4: Deploy the Infrastructure

```bash
terraform apply
```

**What this does:** Creates the actual resources in your Google Cloud project.

**You'll see a confirmation prompt:**
```
Do you want to perform these actions?
  Terraform will perform the actions described above.
  Only 'yes' will be accepted to approve.

  Enter a value: 
```

**Type:** `yes` (and press Enter)

⏳ **Wait 2-3 minutes** for Terraform to create all the resources.

**Success looks like:**
```
Apply complete! Resources: 4 added, 0 changed, 0 destroyed.
```

---

## 🎯 Complete Command Sequence

Here's everything in one place - just copy and paste line by line:

```bash
# 1. Navigate to the Terraform directory
cd tfinfra

# 2. Initialize Terraform
terraform init

# 3. Preview changes (optional)
terraform plan

# 4. Deploy infrastructure
terraform apply
# When prompted, type: yes
```

---

## 📊 What Gets Created

After successful deployment:

| Resource | Name | Details |
|----------|------|---------|
| **VPC Network** | mynetwork | Auto-mode, all regions |
| **Firewall Rule** | mynetwork-allow-http-ssh-rdp-icmp | HTTP, SSH, RDP, ICMP |
| **VM Instance 1** | mynet-vm-1 | us-east1-b, Debian 11 |
| **VM Instance 2** | mynet-vm-2 | europe-west1-d, Debian 11 |

---

## ✔️ Verify Deployment

### In Google Cloud Console:

1. **VPC Networks:**
   - Go to: https://console.cloud.google.com/networking/networks/list
   - Look for: `mynetwork`

2. **Firewall Rules:**
   - Go to: https://console.cloud.google.com/networking/firewalls/list
   - Look for: `mynetwork-allow-http-ssh-rdp-icmp`

3. **VM Instances:**
   - Go to: https://console.cloud.google.com/compute/instances
   - Look for: `mynet-vm-1` and `mynet-vm-2`

### From Command Line:

```bash
# List all VMs
gcloud compute instances list

# Show VPC networks
gcloud compute networks list

# Show firewall rules
gcloud compute firewall-rules list
```

---

## 🧪 Test Connectivity Between VMs

### Step 1: Get the internal IP of VM 2

```bash
gcloud compute instances describe mynet-vm-2 \
  --zone=europe-west1-d \
  --format='get(networkInterfaces[0].networkIP)'
```

**Example output:** `10.132.0.2`

### Step 2: SSH into VM 1

```bash
gcloud compute ssh mynet-vm-1 --zone=us-east1-b
```

### Step 3: Ping VM 2 from VM 1

```bash
ping -c 3 10.132.0.2
# (Replace with the actual IP from Step 1)
```

**Expected result:** ✅ Ping should succeed!

```
3 packets transmitted, 3 received, 0% packet loss
```

---

## 🧹 Clean Up (When Done)

To delete all resources and stop charges:

```bash
cd tfinfra
terraform destroy
# Type 'yes' when prompted
```

---

## ❌ Troubleshooting

### Problem: Still getting "No configuration files"

**Solution:**
```bash
# Check where you are
pwd

# You should be in: .../EARTHING-SYSTEM-HEALTH-MONITORING/tfinfra
# If not, run:
cd tfinfra
```

### Problem: "Error: Could not load plugin"

**Solution:**
```bash
terraform init
```

### Problem: "API not enabled"

**Solution:**
```bash
gcloud services enable compute.googleapis.com
terraform apply
```

### Problem: Permission denied

**Solution:**
```bash
# Check your authentication
gcloud auth list

# Check your project
gcloud config get-value project

# Re-authenticate if needed
gcloud auth login
```

---

## 📚 More Help

If you want more detailed information:

- **Quick solution (this file):** `STEP_BY_STEP.md` ← You are here
- **Detailed walkthrough:** `QUICKSTART.md`
- **Complete reference:** `README.md`
- **Architecture diagrams:** `ARCHITECTURE.md`
- **Automated setup:** Run `./setup.sh`

---

## 💡 Why This Error Happened

```
❌ WRONG:
/home/student/                    ← You were here
└── (no .tf files)

✅ CORRECT:
/home/student/tfinfra/            ← You need to be here
├── provider.tf
├── mynetwork.tf
└── instance/
    ├── main.tf
    └── variables.tf
```

**Remember:** Always run Terraform commands from inside the `tfinfra` directory!

---

## 🎓 Key Takeaway

**The error "No configuration files" means Terraform can't find `.tf` files in your current directory.**

**Solution:** `cd tfinfra` first, then run Terraform commands.

---

## ✨ Quick Reference Card

```bash
cd tfinfra              # Go to Terraform directory (ALWAYS DO THIS FIRST!)
terraform init          # Initialize (first time only)
terraform plan          # Preview changes
terraform apply         # Create resources
terraform show          # Show current state
terraform destroy       # Delete all resources
```

---

**🎉 You're ready to deploy! Start with Step 1 above.**

*Lab: Automating the Deployment of Infrastructure Using Terraform*  
*Last Updated: February 2026*
