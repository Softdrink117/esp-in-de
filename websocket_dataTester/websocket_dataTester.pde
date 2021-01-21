import websockets.*;

WebsocketServer wss;
int port = 81;

int state = 0;
int pState = 0;

boolean up, dn, le, ri, a, b, c, d = false;

void setup()
{
  size(200,200);
  wss = new WebsocketServer(this, port, "");
  background(0);
  
  ResetKeys();
}

void draw()
{
  ReadState();
  
  //ResetKeys();
}

void ReadState()
{
  pState = state;
  state = 0;
  
  // P1 Directions and ABCD
  if(up) state |= 1 << 31;
  if(dn) state |= 1 << 30;
  if(le) state |= 1 << 29;
  if(ri) state |= 1 << 28;
  if(a) state |= 1 << 27;
  if(b) state |= 1 << 26;
  if(c) state |= 1 << 25;
  if(d) state |= 1 << 24;
  
  if(state != pState)
  {
    //println(binary(state));
    byte[] raw = new byte[4];
    raw[0] = byte((state >> 24) & 0xFF);
    raw[1] = byte((state >> 16) & 0xFF);
    raw[2] = byte((state >> 8) & 0xFF);
    raw[3] = byte((state) & 0xFF);
    println("UDLRABCD EFGHS000 UDLRABCD EFGHS000");
    println(binary(raw[0]) + " " + binary(raw[1]) + " " + binary(raw[2]) + " " + binary(raw[3]));
    
    wss.sendMessage(raw);
  }
}

void ResetKeys()
{
  up = false;
  dn = false;
  le = false;
  ri = false;
  a = false;
  b = false;
  c = false;
  d = false;
}

void keyPressed()
{
  if(key == 'A' || key == 'a') le = true;
  if(key == 'W' || key == 'w') up = true;
  if(key == 'S' || key == 's') dn = true;
  if(key == 'D' || key == 'd') ri = true;
  
  if(key == 'Z' || key == 'z') a = true;
  if(key == 'X' || key == 'x') b = true;
  if(key == 'C' || key == 'c') c = true;
  if(key == 'V' || key == 'v') d = true;
}

void keyReleased()
{
  if(key == 'A' || key == 'a') le = false;
  if(key == 'W' || key == 'w') up = false;
  if(key == 'S' || key == 's') dn = false;
  if(key == 'D' || key == 'd') ri = false;
  
  if(key == 'Z' || key == 'z') a = false;
  if(key == 'X' || key == 'x') b = false;
  if(key == 'C' || key == 'c') c = false;
  if(key == 'V' || key == 'v') d = false;
}
