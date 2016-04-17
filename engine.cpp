struct latlong {
    double latitude;
    double longitude;
};

struct Satellite {
    latlong position;
    void move_north(double a) {move(a, 0)};
    void move_south(double a) {move(-a, 0)};
    void move_east(double a)  {move(0, a)};
    void move_west(double a)  {move(0, -a)};
    void move(double, double);
};

void Satellite::move(d_latitude, d_longitude) {
    position.latitude += d_latitude;
    // Can't go more than +-90 latitude
    if (position.latitude > 90)
        position.latitude = 90
    else if (position.latitude < -90)
        position.latitude = -90

    // Modulo 360 after shifting 180 degrees, then unshift
    position.longitude = mod(180+position.longitude+d_longitude,360)-180;
}


double greatcircledist(latlong p1, latlong p2) {
    // Uses Vincenty's formula for computing arc length. Returns central angle,
    // not arc length. Multiply with radius to get arc length.
    double dlongitude = abs(p2.longitude-p1.longitude)
    return arctan(
        sqrt(
            pow(cos(p2.latitude)*sin(dlongitude),2) + 
            pow(cos(p1.latitude)*sin(p2.latitude) - 
                sin(p1.latitude)*cos(p2.latitude)*cos(dlongitude),2)
        )/( sin(p1.latitude)*sin(p2.latitude) + 
            cos(p1.latitude)*cos(p2.latitude)*cos(dlongitude)
        )
    )
}

double maxdist_satellites(vector<Satellite*> s) {
    double maxdist = 0;
    for (Satellite* i:s) {
        for (Satellite* j:s) {
            double d = greatcircledist(i->position, j->position);
            if (d > maxdist)
                maxdist = d;
        }
    }
    return maxdist;
}

Point latlong_mercpoint(latlong p, Point ul, int w, int h) {
    // longitude is linear
    int x = (p.longitude+180)*(w/360) + ul.x;
    // latitude is a bit trickier
    double mercN = log(tan((M_PI/4)+(p.latitude*M_PI/360)));
    int y = (h/2)-(w*mercN/(2*M_PI)) + ul.y;
    return Point{x,y};
}
