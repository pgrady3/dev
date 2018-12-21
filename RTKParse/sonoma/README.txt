sonoma.csv is the raw SwiftNav RTK log. It has not been cleaned or parsed at all

linearizeTrack.m is a matlab script that converts this raw csv into something more useable

sonomaMeters.csv is a map of the track in meters and X, Y, Z. The points are spaced every 5 meters

sonomaLinearized.csv is a 2d map of the track, with only distance and elevation