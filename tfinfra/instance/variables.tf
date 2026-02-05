variable "instance_name" {
  description = "Name of the VM instance"
  type        = string
}

variable "instance_zone" {
  description = "Zone for the VM instance"
  type        = string
}

variable "instance_type" {
  description = "Machine type for the VM instance"
  type        = string
  default     = "e2-micro"
}

variable "instance_network" {
  description = "Network for the VM instance"
  type        = string
}
