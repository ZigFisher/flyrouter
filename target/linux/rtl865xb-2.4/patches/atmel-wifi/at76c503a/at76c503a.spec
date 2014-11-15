%define KERNELVER %(uname -r)
%define PACKVER cvs20030728
# RPM Release number of this version
%define TNREL 1

# Real stuff
Summary:   Linux driver for Atmel AT76C503A based USB WLAN adapters
Name:      at76c503a
Version:   %{PACKVER}
Release:   tn%{TNREL}
Copyright: GPL
Group:     System Environment/Kernel 
Packager:  Tim Niemueller <tim@niemueller.de>
Source:    %{name}.tar.gz
URL:       http://at76c503a.berlios.de/
Prefix:	   %{_prefix}
BuildRequires: kernel-source = %{KERNELVER}
BuildRoot: %{_tmppath}/%{name}-%{PACKVER}

%description
This is another driver for the Atmel AT76C503A based USB WLAN adapters.

%prep
%setup -n %{name}

#if [ ! -e /usr/src/linux-2.4/.config ]; then
#  echo "You need to run 'make menuconfig' once in /usr/src/linux-2.4"
#  echo "Then exit immediately and save the configuration. The "
#  echo "config file is needed to compile hostap. Thanx."
#  exit 1
#fi

%build
echo "=================================================================="
echo "Building atmel driver %{PACKVER} for %{KERNELVER}"
echo "Target is %{_target}"
echo "=================================================================="
sleep 5

make

%install
rm -rf $RPM_BUILD_ROOT

mkdir -p $RPM_BUILD_ROOT/lib/modules/%{KERNELVER}

make DESTDIR=$RPM_BUILD_ROOT install

%clean 
rm -rf $RPM_BUILD_ROOT

%post
/sbin/depmod -a

%postun
/sbin/depmod -a

%files
%defattr(-,root,root)
/lib/modules/%{KERNELVER}/kernel/drivers/usb/*.o

%changelog
* Mon Jul 28 2003 Tim Niemueller <tim@niemueller.de>
- Initial rpm release
- Wrote spec file and compiled for current RedHat 9 kernel

