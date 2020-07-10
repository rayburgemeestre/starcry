pdsh -g computenode systemctl list-units | grep hadoop
scp restart_all.sh root@node1:/tmp/
scp restart_all.sh root@node2:/tmp/
scp restart_all.sh root@node3:/tmp/
ssh root@node1 "chmod +x /tmp/restart_all.sh && /tmp/restart_all.sh"
ssh root@node2 "chmod +x /tmp/restart_all.sh && /tmp/restart_all.sh"
ssh root@node3 "chmod +x /tmp/restart_all.sh && /tmp/restart_all.sh"
chmod +x restart_all.sh && ./restart_all.sh
pdsh -g computenode systemctl list-units | grep hadoop
