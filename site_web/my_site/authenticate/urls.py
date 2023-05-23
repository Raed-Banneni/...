from django.urls import path
from . import views
from django.urls import path, include

urlpatterns = [
    path('api/',views.API_List),
    path('', views.home, name ="home"),
    path('login/', views.login_user, name ='login'),
    path('logout/', views.logout_user, name='logout'),
    path('register/', views.register_user, name='register'),
    path('edit_profile/', views.edit_profile, name='edit_profile'),
    path('change_password/', views.change_password, name='change_password'),
    path('famille/<int:id_famille>/',views.analyse,name='analyse'),
    path('ajouter_famille/<int:pk>/home',views.Addfamille.as_view(),name='ajouter_famille'),
    path('supprimer_famille/<int:pk>/home',views.Deletefamille.as_view(), name='supprimer_famille'),
    path('historique/<int:id_famille>/analyse',views.historique,name='historique'),
    

]

