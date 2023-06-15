/* =======================================================================================================
   NONLINEAR ARRAY THROTTLE
   =======================================================================================================*/

/* Untuk dapat menyesuaikan suara dengan Throttle kendaraan listrik anda, Hanya perlu mengubah angka output Valuenya */

float curveShifting[][2] = {  
  {0, 0} // {input value, output value}
  , {83, 100}
  , {166, 200}//83
  , {250, 300}//84
  , {333, 500}//83
  , {417, 600}//84
  , {500, 700}//83
};

/* =======================================================================================================
   ARRAY INTERPOLATION
   =======================================================================================================*/
/* Credit: http://interface.khm.de/index.php/lab/interfaces-advanced/nonlinear-mapping/ */

int reMap(float pts[][2], int input) {
  int rr;
  float bb, mm;

  for (int nn = 0; nn < 7; nn++) {
    if (input >= pts[nn][0] && input <= pts[nn + 1][0]) {
      mm = ( pts[nn][1] - pts[nn + 1][1] ) / ( pts[nn][0] - pts[nn + 1][0] );
      mm = mm * (input - pts[nn][0]);
      mm = mm +  pts[nn][1];
      rr = mm;
    }
  }
  return (rr);
}
