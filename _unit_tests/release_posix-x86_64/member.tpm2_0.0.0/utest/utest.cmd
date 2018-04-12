@pushd %~dp0
@python utest %*
set ERROR_LEVEL=%ERRORLEVEL%
@popd
exit %ERROR_LEVEL%