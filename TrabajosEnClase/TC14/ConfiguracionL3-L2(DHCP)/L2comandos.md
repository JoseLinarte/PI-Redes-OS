enable
configure terminal
vlan 230
 name ISLA3
exit
interface vlan 230
 no ip address
 ip address 172.16.123.50 255.255.255.240
 no shutdown
exit
ip default-gateway 172.16.123.49
ip dhcp excluded-address 172.16.123.49 172.16.123.50
ip dhcp pool ISLA3
 network 172.16.123.48 255.255.255.240
 default-router 172.16.123.49
 dns-server 8.8.8.8
exit
interface range fa0/1 - 24
 switchport mode access
 switchport access vlan 230
 spanning-tree portfast
 no shutdown
exit
interface gi0/1
 switchport mode trunk
 switchport trunk allowed vlan 230
exit
end
write memory