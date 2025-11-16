#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_info() {
	echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
	echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
	echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
	echo -e "${YELLOW}[!]${NC} $1"
}

# Check if running on macOS
if [[ "$OSTYPE" != "darwin"* ]]; then
	print_error "This setup script currently only supports macOS."
	print_info "For other platforms, please refer to README.md for manual installation instructions."
	exit 1
fi

print_info "Setting up development environment for RISC-V microkernel..."
echo

# Check and install Homebrew
print_info "Checking for Homebrew..."
if ! command -v brew &>/dev/null; then
	print_warning "Homebrew not found. Install Homebrew first..."
	# Suppressed:
	# /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

	# Add Homebrew to PATH for Apple Silicon Macs
	if [[ $(uname -m) == "arm64" ]]; then
		print_warning "You are on Apple Silicon Mac, you also need to add brew to your .zprofile or equivalent and restart your shell."
		# Suppressed:
		# echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> ~/.zprofile
		# eval "$(/opt/homebrew/bin/brew shellenv)"
	fi
	exit 1
else
	print_success "Homebrew is already installed"
fi

# Update Homebrew
print_info "Updating Homebrew..."
brew update &>/dev/null || true

# Install LLVM
print_info "Checking for LLVM toolchain..."
if ! brew list llvm &>/dev/null; then
	print_warning "LLVM not found. Installing LLVM..."
	brew install llvm
	print_success "LLVM installed"
else
	print_success "LLVM is already installed"
fi

# Install LLD (usually comes with LLVM but check separately)
print_info "Checking for LLD linker..."
if ! brew list lld &>/dev/null 2>&1; then
	# LLD is typically included in LLVM package, but try to install if needed
	print_info "LLD is included in LLVM package"
else
	print_success "LLD is available"
fi

# Install QEMU
print_info "Checking for QEMU..."
if ! brew list qemu &>/dev/null; then
	print_warning "QEMU not found. Installing QEMU..."
	brew install qemu
	print_success "QEMU installed"
else
	print_success "QEMU is already installed"
fi

# Verify LLVM tools are accessible
LLVM_PREFIX=$(brew --prefix llvm)
print_info "LLVM installed at: $LLVM_PREFIX"

# Add LLVM to PATH for current session
export PATH="$LLVM_PREFIX/bin:$PATH"

# Check for required LLVM tools
print_info "Verifying LLVM tools..."
REQUIRED_TOOLS=("clang" "lld" "llvm-objdump" "llvm-nm" "llvm-addr2line")
MISSING_TOOLS=()

for tool in "${REQUIRED_TOOLS[@]}"; do
	if command -v "$tool" &>/dev/null || [[ -x "$LLVM_PREFIX/bin/$tool" ]]; then
		print_success "$tool is available"
	else
		MISSING_TOOLS+=("$tool")
		print_error "$tool not found"
	fi
done

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
	print_error "Missing tools: ${MISSING_TOOLS[*]}"
	print_info "Try reinstalling LLVM: brew reinstall llvm"
	exit 1
fi

# Verify QEMU RISC-V support
print_info "Verifying QEMU RISC-V support..."
if command -v qemu-system-riscv32 &>/dev/null; then
	print_success "qemu-system-riscv32 is available"
else
	print_error "qemu-system-riscv32 not found"
	print_info "Try reinstalling QEMU: brew reinstall qemu"
	exit 1
fi

# Download OpenSBI firmware if not present
print_info "Checking for OpenSBI firmware..."
OPENSBI_FILE="opensbi-riscv32-generic-fw_dynamic.bin"
if [[ ! -f "$OPENSBI_FILE" ]]; then
	print_warning "OpenSBI firmware not found. Downloading..."
	curl -LO "https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/$OPENSBI_FILE"
	print_success "OpenSBI firmware downloaded"
else
	print_success "OpenSBI firmware is already present"
fi

# Print PATH configuration instructions
echo
print_success "All dependencies installed successfully!"
echo
print_info "To use LLVM tools in your shell, add the following to your shell configuration:"
print_info "  ~/.zshrc (for zsh) or ~/.bash_profile (for bash)"
echo
echo "  export PATH=\"$LLVM_PREFIX/bin:\$PATH\""
echo
print_info "Or run this command to add it now:"
echo "  echo 'export PATH=\"$LLVM_PREFIX/bin:\$PATH\"' >> ~/.zshrc"
echo

# Check if PATH is already configured
if [[ ":$PATH:" != *":$LLVM_PREFIX/bin:"* ]]; then
	print_warning "LLVM tools are not in your PATH yet."
	print_info "For this session, the PATH has been temporarily updated."
	print_info "To make it permanent, run the command above."
else
	print_success "LLVM tools are already in your PATH"
fi

echo
print_success "Setup complete! You can now run 'make run' to build and run the kernel."
