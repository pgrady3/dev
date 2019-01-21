function [ windowPoints ] = PatrickWindow( velo, power, elapsed)

    startPoints = [];
    stopPoints = [];
    currentState = 0;

    for i = 100:length(velo)
        switch currentState
            case 0  %not started
                if power(i) > 50 && velo(i) > 3
                    fprintf("start accel %d\n", i);
                    currentState = 1;
                end
            case 1  %accelerating
                pass = 1;
                for k = i-25:i-20
                    if power(k) < 5 || velo(k) < 3
                        pass = 0;
                    end
                end

                if pass ~= 1
                    continue
                end

                for k = i-5:i+10
                    if power(k) > 5 || velo(k) < 3
                        pass = 0;
                    end
                end

                if pass ~= 1
                    continue
                end
                
                
                fprintf("start decel %d\n", i);
                currentState = 2;
                startPoints = [startPoints; i];
            case 2  %decelerating
                decel = (velo(i + 30) - velo(i+10)) / (elapsed(i + 30) - elapsed(i+10));
                
                if velo(i) < 1 || power(i+10) > 5% || decel < -0.15
                    currentState = 0;
                    stopPoints = [stopPoints; i];
                    fprintf("end decel %d\n", i);
                end
        end    
    end
    
    windowPoints = [];
    
    for i = 1:length(startPoints)
       start = startPoints(i);
       stop = stopPoints(i);
       dt = elapsed(stop) - elapsed(start);
       
       if dt > 20
          windowPoints = [windowPoints; start, stop]; 
       end
    end
end

