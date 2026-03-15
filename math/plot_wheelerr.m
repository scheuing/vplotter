% v-plotter stepper wheel error (straight line model)

% --- parameters
savesvg = false;
% stepper distance L [mm]
L = 1200;
% stepper wheel radii to analyze [mm]
R_values = [10, 20, 30];
% maximum y position of gondola yMax [mm]
y_max = 800;
% analysis x-y-grid width [mm]
delta_g = 10;

close all

% Loop through each R value
for R_idx = 1:length(R_values)
  R = R_values(R_idx);

  % --- evaluate
  % analysis positions (of gondola) [mm]
  [x,y] = meshgrid(R:delta_g:L-R, delta_g:delta_g:y_max);
  % string lengths without radius impact (left, right) [mm]
  [u,v] = pos2len(x, y, L);
  % back transform into positions with radius impact (xR,yR) [mm]
  [xR, yR] = len2pos(u, v, L, R);

  % position error (Euclidean distance)
  err = sqrt((x-xR).^2 + (y-yR).^2);

  % Create figure for this R value
  figure;
  levels = 0:5:50;  % Contour levels from 0 to 50 mm in steps of 5 mm
  [c, h] = contour(x, y, err, levels);
  view(180, 90)
  title(sprintf('Stepper wheel radius position error [mm] (R = %.0f mm)', R))
  xlabel('x [mm]')
  ylabel('y [mm]')
  set(gca(), "xdir", "reverse")
  colormap('lines')
  clabel(c, h, "fontsize", 12);

  % Save figure as SVG
  if (savesvg)
  	filename = sprintf('../docs/wheelerr_%.0f.svg', R);
  	saveas(gcf, filename);
  	fprintf('Saved: %s\n', filename);
  end
end
