#!/bin/bash
#
# IPCHAINS-FIREWALL V1.7.3
# December 4th, 2000

# -------------------------------------------------- Ipchains Firewall Script -

# Original script by Ian Hall-Beyer <manuka@nerdherd.net>
#
# Contributors:
#
# 1.5
# terminus <cpm@dotquad.com> (ICQ & DHCP, @home testing)
#
# 1.7
# Emannuel Paré <emman@voxtel.com> (timeout values)
# Bill Mote <bill.mote@bigfoot.com> (testing)
# Blair Steenerson <blair@steenerson.com> (lpd block)
#
# ------------------------------------------------------------- Configuration -

# Set a temporary path for various binaries required by this script.
# The original path is saved and restored when the script exits.
# This should fix some of the problems encountered when running 
# from ip-up's restricted shell. Thanks to John Allen for figuring
# this one out. The following system binaries are used:
# 
# ipchains, ifconfig, route, grep, cut 

ORIGPATH=$PATH
export PATH="/sbin:/bin:/usr/bin"

# ---------------------------------------------------------------- Interfaces -

if [ $# -lt 1 ];
 then
  echo "Syntax: $0 <external interface> [internal interface]"
  export PATH=$ORIGPATH
  exit
 elif [ $# = 1 ];
  then
   EXTERNALIF=$1
   INTERNALIF="none"
 else
   EXTERNALIF=$1
   INTERNALIF=$2
fi

# ------------------------------------------------------------- Test ipchains -

FOO=`ipchains -L 2>&1 | grep Incompatible`
if [ "$FOO" = "ipchains: Incompatible with this kernel" ];
   then 
    echo "Kernel not configured for ipchains. Aborting."
    export PATH=$ORIGPATH
    exit
fi

# ------------------------------------------------------------ Grok Addresses -

function grok_interface {
if [ ! -z "`ifconfig $1 2>/dev/null | grep UP`" ];
 then 
  echo "1" 
 else 
  echo "0"
fi
}

function grok_address {
  IP=`ifconfig $1 2>/dev/null| grep inet | cut -d : -f 2 | cut -d \  -f 1`
  MASK=`ifconfig $1 2>/dev/null| grep Mask | cut -d : -f 4`
  NET=`route -n | grep $EXTERNALIF | grep $MASK | grep -w "U" | cut -d\  -f1`
  echo "$IP/$MASK"
}

function grok_net {
  MASK=`ifconfig $1 2>/dev/null| grep Mask | cut -d : -f 4`
  NET=`route -n | grep $1 | grep $MASK | grep -w "U" | cut -d\  -f1`
  echo "$NET/$MASK"

}

echo -n "Checking External Interface..."

if [ "`grok_interface $EXTERNALIF`" != "1" ];
 then 
  echo "$EXTERNALIF unavailable. Aborting."
  export PATH=$ORIGPATH
  exit
 else 
  echo "found $EXTERNALIF"
  echo "External Interface Data:"
  echo "Address: `grok_address $EXTERNALIF`"
  echo "Network: `grok_net $EXTERNALIF`"
fi

echo -n "Checking Internal Interface..."

if [ "$INTERNALIF" = "none" ];
  then
     STANDALONE="1"
     echo "None specified"
     echo "Going to Standalone Mode"                 
  else                      
   if [ "`grok_interface $INTERNALIF`" != "1" ];
    then 
     echo "$INTERNALIF unavailable."
     echo "Going to Standalone Mode"
     STANDALONE="1"
    else
     echo "found $INTERNALIF"
     echo "Internal Interface Data:"
     echo "Address: `grok_address $INTERNALIF`"
     echo "Network: `grok_net $INTERNALIF`"
     STANDALONE="0"
   fi
fi

if [ "$STANDALONE" = "0" ];
  then 
     echo -n "Checking internal interface for RFC1918"
     declare -i DOTQUAD1                                                                     
     declare -i DOTQUAD2                                                                     

     DOTQUAD1=`grok_net $INTERNALIF | cut -f1 -d.`                                              
     DOTQUAD2=`grok_net $INTERNALIF | cut -f2 -d.`                                              
     
     if [ "$DOTQUAD1" = "10" ]; then                                  
         MASQUERADE="1"
         echo "...Class A found"      
         echo "Going to Masq Mode"
      elif [ "$DOTQUAD1" = "192" -a "$DOTQUAD2" = "168" ]; then          
         MASQUERADE="1" 
         echo "...Class C found"
         echo "Going to Masq Mode"
      elif [ "$DOTQUAD1" = "172" -a $DOTQUAD2 -gt 15 -a $DOTQUAD2 -lt 32 ]; then
         MASQUERADE="1" 
         echo "...Class B found"
         echo "Going to Masq Mode"
      else
         echo "...None found"
         echo "Going to Routable Mode"
         MASQUERADE="0"
     fi
fi                                                                                           

function flush_rulesets {

# No parameters

echo -n "Flushing rulesets.."

# Incoming packets from the outside network
ipchains -F input
echo -n "."

# Outgoing packets from the internal network
ipchains -F output   
echo -n "."

# Forwarding/masquerading
ipchains -F forward
echo -n "."

echo "Done!"
}

function masq_setup {

# Parameters: internalnet, externalnet

echo -n "Masquerading.."

# don't masquerade internal-internal traffic
ipchains -A forward -s $1 -d $1 -j ACCEPT
echo -n "."

# don't Masquerade external interface direct
ipchains -A forward -s $2 -d 0/0 -j ACCEPT 
echo -n "."

# masquerade all internal IP's going outside
ipchains -A forward -s $1 -d 0/0 -j MASQ

echo -n "."

# set Default rule on forward chain to Deny
ipchains -P forward DENY
echo -n "."
}

function routable_setup {

# Parameters: internalnet, externalnet

echo -n "Forwarding.."

# Forward internal-internal traffic
ipchains -A forward -s $1 -d $1 -j ACCEPT
echo -n "."

# Forward external interface direct
ipchains -A forward -s $2 -d $0/0 -j ACCEPT 
echo -n "."

# Forward internal network going outside
ipchains -A forward -s $1 -d $1 -j ACCEPT
echo -n "."

# Forward external network going inside    
ipchains -A forward -s $2 -d $1 -j ACCEPT
echo -n "."

echo "Done!"

}


function loopback {

# Parameters: No parameters

echo -n "Loopback..."

ipchains -A input -i lo -s 0/0 -d 0/0 -j ACCEPT
ipchains -A output -i lo -s 0/0 -d 0/0 -j ACCEPT
echo "Done!"

}

function tos_setup {

# Parameters: No parameters

echo -n "TOS flags.."

ipchains -A output -p tcp -d 0/0 www -t 0x01 0x10
ipchains -A output -p tcp -d 0/0 telnet -t 0x01 0x10   
ipchains -A output -p tcp -d 0/0 ftp -t 0x01 0x10
echo -n "..."

# Set ftp-data for maximum throughput
ipchains -A output -p tcp -d 0/0 ftp-data -t 0x01 0x08
echo -n "."

echo "Done!"

}

function outbound_setup {

# Parameters: internalnet

echo -n "Outbound connections.."
ipchains -A input -s $1 -d 0/0 -j ACCEPT   
ipchains -A output -s $1 -d 0/0 -j ACCEPT
echo -n ".."

echo "Done!"

}

function standard_rules {

# Parameters: target, standalone, internalif

# ---------------------------------------------------------- Trusted Networks -
# Add in any rules to specifically allow connections from hosts/nets that
# would otherwise be blocked.

# echo -n "Trusted Networks.."

# ipchains -A input -s [trusted host/net] -d $1 <ports> -j ACCEPT 
# echo -n "."

# echo "Done!"

# ----------------------------------------------------------- Banned Networks -
# Add in any rules to specifically block connections from hosts/nets that
# have been known to cause you problems. These packets are logged.

# echo -n "Banned Networks.."

# This one is generic
# ipchains -A input -l -s [banned host/net] -d $1 <ports> -j DENY
# echo -n "."

# This one blocks ICMP attacks
# note: doesn't seem to work anymore  -neuro
# ipchains -A input -l -b -i $LOCALIF -p icmp -s [host/net] -d $1 -j DENY
# echo -n "."

# Siteban START
# Siteban END

echo "Done!"

# ------------------------------------------------------ @home-specific rules -
# This @home stuff is pretty specific to me (terminus).  I get massive port
# scans from my neighbors and from pokey admins at @home, so I just got harsh
# and blocked all their stuff, with a few exceptions, listed below.
#
# If someone out there finds out the ip ranges of JUST tci@home, let me know
# so i don't end up blocking ALL cablemodems like it's doing now.

echo -n "Cable Modem Nets.."

# so we can check mail, use the proxy server, hit @home's webpage.
# you will want to set these to your local servers, and uncomment them

# ipchains -A input -p tcp -s ha1.rdc1.wa.home.com -d $1 1023:65535 -j ACCEPT
# ipchains -A input -p tcp -s mail.tcma1.wa.home.com -d $1 1023:65535 -j ACCEPT
# ipchains -A input -p tcp -s www.tcma1.wa.home.com -d $1 1023:65355 -j ACCEPT
# ipchains -A input -p tcp -s proxy.tcma1.wa.home.com -d $1 1023:65535  -j ACCEPT
# echo -n "...."

# so we can resolve the above hostnames, allow dns queries back to us
# ipchains -A input -p tcp -s ns1.home.net -d $1 1023:65535 -j ACCEPT
# ipchains -A input -p tcp -s ns2.home.net -d $1 1023:65535 -j ACCEPT
# ipchains -A input -p udp -s ns1.home.net -d $1 1023:65535 -j ACCEPT
# ipchains -A input -p udp -s ns2.home.net -d $1 1023:65535 -j ACCEPT
# echo -n ".."

# linux ipchains building script page (I think)
ipchains -A input -p tcp -s 24.128.61.117 -d $1 1023:65535 -j  ACCEPT
# echo -n "."

# Non-@home users may want to leave this uncommented, just to block all
# the wannabe crackers. Add any @home hosts you want to allow BEFORE this line.

# Blast all other @home connections into infinity and log them.
#ipchains -A input -l -s 24.0.0.0/8 -d $1 -j DENY
# echo -n "."

# Nuke any connections from @home's portscanners (ops-scan.home.com)
ipchains -A input -l -s 24.0.84.130 -d $1 -j DENY
echo -n "."

echo "Done!"

# ---------------------------- Specific port blocks on the external interface -
# This section blocks off ports/services to the outside that have
# vulnerabilities. This will not affect the ability to use these services
# within your network. 

echo -n "Port Blocks.."
 
# NetBEUI/Samba
ipchains -A input -p tcp -s 0/0 -d $1 137:139 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 137:139 -j DENY
echo -n "."

# CIFS (Win2K)
ipchains -A input -p tcp -s 0/0 -d $1 445 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 445 -j DENY

# Line Printer Daemon (courtesy of Blair Steenerson <blair@steenerson.com>)
ipchains -A input -p tcp -s 0/0 -d $1 515 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 515 -j DENY
echo -n "."

# Microsoft SQL
ipchains -A input -p tcp -s 0/0 -d $1 1433 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 1433 -j DENY
echo -n "."

# Postgres SQL

ipchains -A input -p tcp -s 0/0 -d $1 5432 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 5432 -j DENY
echo -n "."

# Network File System
ipchains -A input -p tcp -s 0/0 -d $1 2049 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 2049 -j DENY
echo -n "."

# MySQL
ipchains -A input -p tcp -s 0/0 -d $1 3306 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 3306 -j DENY
echo -n "."

# X Displays
ipchains -A input -p tcp -s 0/0 -d $1 5999:6010 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 5999:6010 -j DENY
echo -n "."

# X Font Server :0-:2-
ipchains -A input -p tcp -s 0/0 -d $1 7100:7101 -j DENY
ipchains -A input -p udp -s 0/0 -d $1 7100:7101 -j DENY
echo -n "."

# Back Orifice (logged)
ipchains -A input -l -p tcp -s 0/0 -d $1 31337 -j DENY
ipchains -A input -l -p udp -s 0/0 -d $1 31337 -j DENY
echo -n "."

# NetBus (logged)
ipchains -A input -l -p tcp -s 0/0 -d $1 12345:12346 -j DENY
ipchains -A input -l -p udp -s 0/0 -d $1 12345:12346 -j DENY
echo -n "."

echo "Done!"

# --------------------------------------------------- High Unprivileged ports -
# These are opened up to allow sockets created by connections allowed by 
# ipchains

echo -n "High Ports.."

ipchains -A input -p tcp -s 0/0 -d $1 1023:65535 -j ACCEPT
ipchains -A input -p udp -s 0/0 -d $1 1023:65535 -j ACCEPT
echo -n "."

echo "Done!"

# ------------------------------------------------------------ Basic Services -

echo -n "Services.."

# ftp-data (20) and ftp (21)
#ipchains -A input -p tcp -s 0/0 -d $1 20 -j ACCEPT
#ipchains -A input -p tcp -s 0/0 -d $1 21 -j ACCEPT
#echo -n ".."

# ssh (22)
ipchains -A input -p tcp -s 0/0 -d $1 22 -j ACCEPT
echo -n "."

# telnet (23)
ipchains -A input -p tcp -s 0/0 -d $1 23 -j ACCEPT
# echo -n "."

# smtp (25)
# ipchains -A input -p tcp -s 0/0 -d $1 25 -j ACCEPT
# echo -n "."

# DNS (53)
#ipchains -A input -p tcp -s 0/0 -d $1 53 -j ACCEPT
#ipchains -A input -p udp -s 0/0 -d $1 53 -j ACCEPT
#echo -n ".."

if [ "$2" != "1" ]; then
#  DHCP on LAN side (to make @Home DHCP work) (67/68)
#  ipchains -A input -i $3 -p udp -s 0/0 -d 255.255.255.255/24 67 -j ACCEPT
#  ipchains -A output -i $3 -p udp -s 0/0 -d 255.255.255.255/24 68 -j ACCEPT
echo -n ".."
fi

# http (80)
#ipchains -A input -p tcp -s 0/0 -d $1 80 -j ACCEPT
echo -n "."

# kerberos (88)
#ipchains -A input -p tcp -s 0/0 -d $1 88 -j ACCEPT


# POP-3 (110)
# ipchains -A input -p tcp -s 0/0 -d $1 110 -j ACCEPT
# echo -n "."

# identd (113)
#ipchains -A input -p tcp -s 0/0 -d $1 113 -j ACCEPT
#echo -n "."

# nntp (119)
# ipchains -A input -p tcp -s 0/0 -d $1 119 -j ACCEPT
# echo -n "."

# LDAP (389)
# ipchains -A input -p tcp -s 0/0 -d $1 389 -j ACCEPT
# echo -n "."

# https (443)
# ipchains -A input -p tcp -s 0/0 -d $1 443 -j ACCEPT
# echo -n "."

# ICQ Services (it's a server service) (4000)
#ipchains -A input -p tcp -s 0/0 -d $1 4000 -j ACCEPT
echo -n "."

echo "Done!"

# ---------------------------------------------------------------------- ICMP -

echo -n "ICMP Rules.."

# Use this to deny ICMP attacks from specific addresses
# ipchains -A input -b -i $EXTERNALIF -p icmp -s <address> -d 0/0 -j DENY
# echo -n "."

# Allow incoming ICMP
ipchains -A input -p icmp -s 0/0 -d $1 -j ACCEPT
ipchains -A input -p icmp -s 0/0 -d $1 -j ACCEPT
echo -n ".."

# Allow outgoing ICMP
ipchains -A output -p icmp -s $1 -d 0/0 -j ACCEPT
ipchains -A output -p icmp -s $1 -d 0/0 -j ACCEPT
echo -n "...."

echo "Done!"

}

function set_policy {

# Parameters: no parameters

ipchains -A input -j DENY
ipchains -A output -j ACCEPT

}

function set_timeouts {

# Parameters: no parameters

echo -n "Setting Ipchains timeout for tcp tcpfin udp.. "                              
ipchains -M -S 7200 150 1000                                                      
echo "Done!"                                                                  

}

flush_rulesets

if [ "$STANDALONE" = "1" ];
 then 
  TARGET=`grok_net $EXTERNALIF`
 else
  if [ "$MASQUERADE" = "1" ];
   then
    TARGET=`grok_address $EXTERNALIF`
    INTERNALNET=`grok_net $INTERNALIF`
    EXTERNALNET=`grok_net $EXTERNALIF`
    outbound_setup $INTERNALNET
    set_timeouts
    masq_setup $INTERNALNET $TARGET
   else
    TARGET=`grok_net $INTERNALIF`
    INTERNALNET=`grok_net $INTERNALIF`
    EXTERNALNET=`grok_net $EXTERNALIF`
    outbound_setup $INTERNALNET
    routable_setup $INTERNALNET $EXTERNALNET
  fi
fi

tos_setup
loopback
standard_rules $TARGET $STANDALONE $INTERNALIF
#configure_vpn $INTERNALNET $EXTERNALNET 
set_policy


export PATH=$ORIGPATH
exit
