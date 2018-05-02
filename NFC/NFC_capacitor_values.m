
frequency = 13560000; %NFC frequency used 
L = 0.617e-6; %from the online NFC calculator for the Transionospheric 

%f = 1/(2*pi*sqrt(L*C)); %resonance equation

%f*f = 1/(4*pi*pi*L*C); %square both sides 
C = 1/(4*pi*pi*L*frequency*frequency) %re-write to get capacitance

