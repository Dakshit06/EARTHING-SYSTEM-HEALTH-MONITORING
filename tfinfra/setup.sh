#!/bin/bash
# Quick start script for Terraform infrastructure deployment

set -e

echo "=========================================="
echo "Terraform Infrastructure Setup"
echo "=========================================="
echo ""

# Check if we're in the correct directory
if [ ! -f "provider.tf" ]; then
    echo "Error: Not in the tfinfra directory!"
    echo ""
    echo "Please run this script from the tfinfra directory:"
    echo "  cd tfinfra"
    echo "  ./setup.sh"
    exit 1
fi

# Check if terraform is installed
if ! command -v terraform &> /dev/null; then
    echo "Error: Terraform is not installed!"
    echo ""
    echo "Please install Terraform first:"
    echo "  https://www.terraform.io/downloads"
    exit 1
fi

echo "Step 1: Checking Terraform version..."
terraform version
echo ""

echo "Step 2: Formatting Terraform files..."
terraform fmt
echo ""

echo "Step 3: Initializing Terraform..."
terraform init
echo ""

echo "Step 4: Validating configuration..."
terraform validate
echo ""

echo "Step 5: Creating execution plan..."
terraform plan
echo ""

echo "=========================================="
echo "Setup complete!"
echo ""
echo "To deploy the infrastructure, run:"
echo "  terraform apply"
echo ""
echo "To destroy the infrastructure later, run:"
echo "  terraform destroy"
echo "=========================================="
