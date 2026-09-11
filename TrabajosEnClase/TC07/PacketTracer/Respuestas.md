# Activity 3.5.1 — Basic VLAN Configuration
## Preguntas y Respuestas

---

**Q: What ports are currently assigned to the four VLANs you have created?**

R: Al crear las VLANs, ningún puerto queda asignado todavía. Todos los puertos siguen en la VLAN 1 hasta que se asignan manualmente con `switchport access vlan`.

---

**Q: Which ports are assigned to VLAN 10?**

R: El puerto Fa0/11.

---

**Q: Ping from host PC2 to host PC1 (172.17.10.21). Is the ping attempt successful?**

R: No. PC2 y PC1 están en VLANs y subredes diferentes, y no hay un dispositivo de Capa 3 que las conecte.

---

**Q: Ping from host PC2 to the switch VLAN 99 IP address 172.17.99.12. Is the ping attempt successful?**

R: No. PC2 está en la VLAN 20 y la IP 172.17.99.12 pertenece a la VLAN 99, así que tampoco hay comunicación sin un router.

---

**Q: Ping from host PC2 to host PC5 (172.17.20.25). Is the ping attempt successful?**

R: Sí. PC2 y PC5 están en la misma VLAN  y en la misma subred, por lo que se pueden comunicar directamente.

---

**Q: Ping from host PC2 to host PC1 (después de mover el puerto Fa0/11 a VLAN 20).**

R: No, todavía falla. Aunque el puerto de PC1 ya pertenece a la VLAN 20, PC1 sigue teniendo una IP de la subred 172.17.10.0/24, que no coincide con la VLAN 20.

---

**Q: Once again, ping from host PC2 to host PC1, using the newly assigned IP address. Is the ping attempt successful?**

R: Sí. Después de cambiar la IP de PC1 a 172.17.20.21, el ping funciona correctamente.

---

**Q: Why was this attempt successful?**

R: Porque ahora PC1 y PC2 están en la misma VLAN y en la misma subred. Al coincidir tanto el puerto como la dirección IP, ya pueden comunicarse sin necesidad de un router.
