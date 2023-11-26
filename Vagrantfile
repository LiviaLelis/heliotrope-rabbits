Vagrant.configure("2") do |config|
  # Use Ubuntu as the base image
  config.vm.box = "ubuntu/bionic64"

  # Provisioning script to install necessary packages
  config.vm.provision "shell", inline: <<-SHELL
    sudo apt-get update
    sudo apt-get install -y build-essential   # Installs GCC, G++
    sudo apt-get install -y cmake             # Installs CMake
    sudo apt-get install -y libomp-dev        # Installs OpenMP
    sudo apt-get install -y libopenmpi-dev    # Installs MPI
    sudo apt-get install -y ninja-build       # Installs Ninja
  SHELL

  # Configure a shared folder. The host directory (.) will be mounted to /vagrant in the VM.
  config.vm.synced_folder ".", "/vagrant"

  # (Optional) Configuration for network, memory, CPU, etc.
  # config.vm.network "private_network", ip: "192.168.56.101"
  # config.vm.provider "virtualbox" do |vb|
  #   vb.memory = "2048"
  #   vb.cpus = 2
  # end
end
