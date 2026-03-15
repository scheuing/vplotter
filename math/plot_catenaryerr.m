% v-plotter stepper catenary error

% --- parameters
savesvg = false;
% stepper distance L [mm]
L = 1200;
% ratio of gondola mass and string linear mass density [m]
q_values = [15, 10, 5];
% maximum y position of gondola yMax [mm]
y_max = 800;
% analysis x-y-grid width [mm]
delta_g = 25;

close all

% Loop through each q value
for q_idx = 1:length(q_values)
  q = q_values(q_idx);

  % --- evaluate
  % analysis positions (of gondola) [mm]
  [x,y] = meshgrid(delta_g:delta_g:L-delta_g, delta_g:delta_g:y_max);
  % string lengths for ideal model (left, right) [m]
  [uIdeal,vIdeal] = pos2len(x, y, L);
  % string lengths with wheel radius and catenary correction (left, right) [mm]
  [uCat,vCat] = pos2len(x, y, L, 0, q*1000);

  % position error (Euclidean string length distance)
  err = sqrt((uIdeal-uCat).^2 + (vIdeal-vCat).^2);

  % Create figure for this R value
  figure;
  levels = [5,10,20,50,100];
  [c, h] = contour(x, y, err, levels);
  view(180, 90)
  title(sprintf('Catenary string length error [mm] for m_G/\\rho_A = %3.0f m', q), "fontsize", 12)
  xlabel('x [mm]')
  ylabel('y [mm]')
  set(gca(), "xdir", "reverse")
  colormap('lines')
  clabel(c, h, "fontsize", 12);

  % Save figure as SVG
  if (savesvg)
    filename = sprintf('../docs/catenaryerr_%.0f.svg', q);
  	saveas(gcf, filename);
  	fprintf('Saved: %s\n', filename);
  end
end
