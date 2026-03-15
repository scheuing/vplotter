% v-plotter sensitivity plot (ideal straight line model)

% --- parameters
savesvg = false;
% stepper distance L [mm]
L = 1200;
% string step size delta_s [mm]
delta_s = 1;
% maximum y position of gondola yMax [mm]
y_max = 800;
% analysis x-y-grid width [mm]
delta_g = 10;

% --- evaluate
% analysis positions (of gondola) [mm]
[x,y] = meshgrid(delta_g:delta_g:L-delta_g, delta_g:delta_g:y_max);
% transformed string lengths (left, right) [mm]
[u,v] = pos2len(x, y, L);

% idea:
% drawing a "square" of +delta_s in (u,v)-domain
% re-transform "square points" into (x,y)-domain
% evaluate range of "square points" in x and y direction
[xN, yN] = len2pos(u, v, L);
[xE, yE] = len2pos(u, v+delta_s, L);
[xW, yW] = len2pos(u+delta_s, v, L);
[xS, yS] = len2pos(u+delta_s, v+delta_s, L);

deltax = xW-xE;
deltay = yS-yN;

close all
figure;
[c1,h1] = contour(x,y,deltax/delta_s,1:0.2:2);
view(180,90)
title('\Deltax/\Deltas')
xlabel('x [mm]')
ylabel('y [mm]')
set (gca (), "xdir", "reverse")
colormap('lines')
clabel (c1, h1, 1:0.2:2, "fontsize", 12);
if (savesvg)
  saveas(gcf, sprintf('../docs/sensitivity_x.svg'));
end

figure;
[c2,h2] = contour(x,y,deltay/delta_s,[1.5,2,2.8,5,10]);
view(180,90)
title('\Deltay/\Deltas')
xlabel('x [mm]')
ylabel('y [mm]')
set (gca (), "xdir", "reverse")
colormap('lines')
clabel (c2, h2, [1.5,2,2.8,5,10], "fontsize", 12);
if (savesvg)
  saveas(gcf, sprintf('../docs/sensitivity_y.svg'));
end
