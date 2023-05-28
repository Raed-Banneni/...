
from django.contrib import admin
from django.urls import path, include
from django.conf import settings
from django.conf.urls.static import static
from rest_framework import routers




urlpatterns = [
    path('admin/', admin.site.urls),
    path('', include('authenticate.urls')),
    
  
]+static(settings.MEDIA_URL,document_root=settings.MEDIA_ROOT)
