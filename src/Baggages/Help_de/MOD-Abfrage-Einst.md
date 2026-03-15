### Abfrage-Einstellungen

Im Modbus-Gateway werden alle Kanäle nacheinander abgefragt. Nachdem alle Kanäle abgefragt wurden, beginnt der Vorgang direkt wieder von vorne. 

#### Zeit zwischen zwei Modbus Register Abfragen 
Hier wird die Zeit eingestellt, die gewartet wird, nachdem ein Kanal auf die Anfrage des Gateways geantwortet hat. 

Das kann notwendig werden, falls einer der Modbus-Teilnehmer nicht schnell genug auf Anfragen des Gateways reagiert.


#### Zeit zwischen zwei Abfragezyklen

Hier wird die Zeit eigestellt, die gewartet wird, nachdem der letzte Kanal abgefragt wurde und ein neuer Abfrage-Zyklus aller Kanäle startet. 